#include "listingtextrenderer.h"
#include "listingrenderercommon.h"
#include "../themeprovider.h"
#include <cmath>
#include <QApplication>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QPalette>
#include <QPainter>

ListingTextRenderer::ListingTextRenderer(const QFont &font, REDasm::DisassemblerAPI *disassembler): REDasm::ListingRenderer(disassembler), m_font(font), m_fontmetrics(font), m_cursoractive(false)
{
    m_rgxwords.setPattern(ListingRendererCommon::wordsPattern());
    m_textoption.setWrapMode(QTextOption::NoWrap);
}

REDasm::ListingCursor::Position ListingTextRenderer::hitTest(const QPointF &pos, int firstline)
{
    REDasm::ListingCursor::Position cp;
    cp.first = firstline + std::floor(pos.y() / m_fontmetrics.height());
    cp.second = -1;

    REDasm::RendererLine rl;
    this->getRendererLine(cp.first, rl);
    std::string s = rl.text;

    for(size_t i = 0; i < s.length(); i++)
    {
        QRect r = m_fontmetrics.boundingRect(QString::fromStdString(s.substr(0, i + 1)));

        if(!r.contains(QPoint(pos.x(), r.y())))
            continue;

        cp.second = i;
        break;
    }

    if(cp.second == -1)
        cp.second = static_cast<int>(s.length() - 1);

    return cp;
}

std::string ListingTextRenderer::getWordUnderCursor(const QPointF &pos, int firstline, int *p)
{
    REDasm::ListingCursor::Position cp = this->hitTest(pos, firstline);

    REDasm::RendererLine rl;
    this->getRendererLine(cp.first, rl);

    return this->findWordUnderCursor(rl.text, cp, p);
}

ListingTextRenderer::Range ListingTextRenderer::wordHitTest(const QPointF &pos, int firstline)
{
    int p = -1;
    std::string word = this->getWordUnderCursor(pos, firstline, &p);
    m_cursor->setWordUnderCursor(word);
    return std::make_pair(p, static_cast<int>(p + word.length() - 1));
}

void ListingTextRenderer::highlightWordUnderCursor()
{
    REDasm::RendererLine rl;

    this->getRendererLine(m_cursor->currentLine(), rl);
    m_cursor->setWordUnderCursor(this->findWordUnderCursor(rl.text, m_cursor->currentPosition()));
}

void ListingTextRenderer::toggleCursor() { m_cursoractive = !m_cursoractive; }
void ListingTextRenderer::enableCursor() { m_cursoractive = true; }
void ListingTextRenderer::disableCursor() { m_cursoractive = false; }

void ListingTextRenderer::renderLine(const REDasm::RendererLine &rl)
{
    QTextDocument textdocument;
    textdocument.setDocumentMargin(0);
    textdocument.setUndoRedoEnabled(false);
    textdocument.setDefaultTextOption(m_textoption);
    textdocument.setDefaultFont(m_font);

    ListingRendererCommon lrc(&textdocument, m_document);
    lrc.insertText(rl, m_cursoractive);

    QPainter* painter = reinterpret_cast<QPainter*>(rl.userdata);
    QRect rvp = painter->viewport();
    rvp.setY(rl.index * m_fontmetrics.height());
    rvp.setHeight(m_fontmetrics.height());

    painter->save();
        painter->translate(rvp.topLeft());
        textdocument.setTextWidth(rvp.width());
        textdocument.drawContents(painter);
    painter->restore();
}

std::string ListingTextRenderer::findWordUnderCursor(const std::string &s, const REDasm::ListingCursor::Position &cp, int* pos)
{
    QRegularExpressionMatchIterator it = m_rgxwords.globalMatch(QString::fromStdString(s));

    while(it.hasNext())
    {
        QRegularExpressionMatch match = it.next();

        if((cp.second < match.capturedStart()) || (cp.second > match.capturedEnd()))
            continue;

        if(pos)
            *pos = match.capturedStart();

        return match.captured().toStdString();
    }

    return std::string();
}
