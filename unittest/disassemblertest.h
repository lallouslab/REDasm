#ifndef DISASSEMBLERTEST_H
#define DISASSEMBLERTEST_H

#include <map>
#include <functional>
#include <QStringList>
#include "redasm/disassembler/disassembler.h"

class DisassemblerTest
{
    private:
        typedef std::function<void()> TestCallback;
        typedef std::map<std::string, TestCallback> TestList;
        typedef std::pair<std::string, TestCallback> TestItem;

    public:
        REDasm::SimpleEvent done;

    public:
        DisassemblerTest();
        void runTests();

    private:
        static std::string replaceAll(std::string str, const std::string& from, const std::string& to);
        static QByteArray readFile(const QString& file);
        void runCurrentTest(const TestCallback& cb);

    private:
        void testVBEvents(const std::map<address_t, std::string>& vbevents);

    private: // Tests
        void testCavia();
        void testCM01();
        void testOllyDump();
        void testSCrack();
        void testVB5CrackMe();
        void testIoliARM();
        void testTn11();

    private:
        TestList m_tests;
        QByteArray m_data;
        std::unique_ptr<REDasm::Disassembler> m_disassembler;
        REDasm::ListingDocument* m_document;
};

#endif // DISASSEMBLERTEST_H
