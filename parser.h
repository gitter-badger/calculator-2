#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <string>
#include <list>
#include <map>

namespace parser
{
class CCommonException: public std::exception
{
    public:
        CCommonException(const char* message): m_what(message) {}
        const char* what() const throw() { return m_what; }

    private:
        const char* m_what;
};

class CParserException: public CCommonException
{
    public:
        CParserException(const char* message): CCommonException(message) {}
};

class CRuntimeException: public CCommonException
{
    public:
        CRuntimeException(const char* message): CCommonException(message) {}
};

enum EParserState
{
    EPS_UNDEFINED,
    EPS_ERROR,
    EPS_READY
};

#define TYPES   TOKEN(ETYPE_UNDEFINED) \
    TOKEN(ETYPE_INTEGER) \
    TOKEN(ETYPE_FLOAT) \
    TOKEN(ETYPE_BOOLEAN) \
    TOKEN(ETYPE_STRING)

class CValue
{
    public:
        enum EType
        {
#define TOKEN(x) x,
            TYPES
#undef TOKEN
        };

        CValue(): m_type(ETYPE_UNDEFINED) {}
        CValue(long long int value): m_type(ETYPE_INTEGER) { m_value.i = value; }
        CValue(long double value): m_type(ETYPE_FLOAT) { m_value.f = value; }
        CValue(bool value): m_type(ETYPE_BOOLEAN) { m_value.b = value; }
        CValue(const std::string& value): m_type(ETYPE_STRING), m_string(value) { m_value.s = m_string.c_str(); }
        CValue& operator=(const CValue& right);
        CValue(const CValue& from);

        EType type() const { return m_type; }
        long long int get_integer() const;
        long double get_float() const;
        bool get_boolean() const;
        const std::string& get_string() const;
        CValue cast_to(EType type) const;

        static EType top_type(EType a, EType b);

    private:
        union UValue
        {
            long long int i;
            long double f;
            bool b;
            const char* s;
        };

        EType m_type;
        UValue m_value;
        std::string m_string;
};

CValue operator<(const CValue& a, const CValue& b);
CValue operator<=(const CValue& a, const CValue& b);
CValue operator==(const CValue& a, const CValue& b);
CValue operator!=(const CValue& a, const CValue& b);
CValue operator>(const CValue& a, const CValue& b);
CValue operator>=(const CValue& a, const CValue& b);
CValue operator+(const CValue& a, const CValue& b);
CValue operator-(const CValue& a, const CValue& b);
CValue operator/(const CValue& a, const CValue& b);
CValue operator*(const CValue& a, const CValue& b);

class CVariable
{
    public:
        CVariable(): m_name(""), m_value() {}
        CVariable(const std::string& name): m_name(name) {}
        CVariable(const std::string& name, const CValue& value): m_name(name), m_value(value) {}
        const CValue& value() const { return m_value; }

    private:
        std::string m_name;
        CValue m_value;
};

class CVariables
{
    public:
        CVariables() {}
        void add_variable(const std::string& name, const CValue& value);
        const CVariable& get_variable(const std::string& name) const;

    private:
        typedef std::map<std::string, CVariable> variables_map_t;

        variables_map_t m_variables;
};

class CTreeNode
{
    public:
        virtual CValue value() const = 0;
};

class CLeafTreeNode: public CTreeNode
{
};

class CConstantTreeNode: public CLeafTreeNode
{
    public:
        CConstantTreeNode(long long int value): m_value(value) {}
        CConstantTreeNode(long double value): m_value(value) {}
        CConstantTreeNode(bool value): m_value(value) {}
        CConstantTreeNode(const std::string& value): m_value(value) {}
        CValue value() const { return m_value; }

    private:
        CValue m_value;
};

class CVariableTreeNode: public CLeafTreeNode
{
    public:
        CVariableTreeNode(const std::string& name): m_variable(name) {}
        CValue value() const { return m_variable.value(); }

    private:
        CVariable m_variable;
};

class COperatorTreeNode: public CTreeNode
{
    public:
        virtual void add_argument(const CTreeNode* node) = 0;

    protected:
        typedef std::list<const CTreeNode*> arguments_list_t;

        arguments_list_t m_arguments;
};

class CUnaryOperator: public COperatorTreeNode
{
    public:
        void add_argument(const CTreeNode* node)
        {
            if (!m_arguments.empty())
            {
                throw CParserException("So much arguments for unary operator");
            }
            m_arguments.push_back(node);
        }
};

class CBinaryOperator: public COperatorTreeNode
{
    public:
        void add_argument(const CTreeNode* node)
        {
            if (2 < m_arguments.size())
            {
                throw CParserException("So much arguments for binary operator");
            }
            m_arguments.push_back(node);
        }
};

class CAddOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() + m_arguments.back()->value();
        }
};

class CSubOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() - m_arguments.back()->value();
        }
};

class CDivOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() / m_arguments.back()->value();
        }
};

class CMulOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() * m_arguments.back()->value();
        }
};

class CLessOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() < m_arguments.back()->value();
        }
};

class CLessOrEqualOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() <= m_arguments.back()->value();
        }
};

class CEqualOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() == m_arguments.back()->value();
        }
};

class CNotEqualOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() != m_arguments.back()->value();
        }
};

class CGreaterOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() > m_arguments.back()->value();
        }
};

class CGreaterOrEqualOperator: public CBinaryOperator
{
    public:
        CValue value() const
        {
            return m_arguments.front()->value() >= m_arguments.back()->value();
        }
};

class CTree
{
    public:
        CTree(): m_root(NULL) {}

    private:
        CTreeNode* m_root;
};

class CTreeNodeFactory
{
    public:
        static CConstantTreeNode* createConstantNode(long long int value) { return new CConstantTreeNode(value); }
        static CConstantTreeNode* createConstantNode(long double value) { return new CConstantTreeNode(value); }
        static CConstantTreeNode* createConstantNode(bool value) { return new CConstantTreeNode(value); }
        static CConstantTreeNode* createConstantNode(const std::string& value) { return new CConstantTreeNode(value); }
        static CVariableTreeNode* createVariableNode(const std::string& name) { return new CVariableTreeNode(name); }
        static CLessOperator* createLessOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CLessOperator>(left, right); }
        static CLessOrEqualOperator* createLessOrEqualOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CLessOrEqualOperator>(left, right); }
        static CEqualOperator* createEqualOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CEqualOperator>(left, right); }
        static CNotEqualOperator* createNotEqualOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CNotEqualOperator>(left, right); }
        static CGreaterOperator* createGreaterOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CGreaterOperator>(left, right); }
        static CGreaterOrEqualOperator* createGreaterOrEqualOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CGreaterOrEqualOperator>(left, right); }
        static CAddOperator* createAddOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CAddOperator>(left, right); }
        static CSubOperator* createSubOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CSubOperator>(left, right); }
        static CMulOperator* createMulOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CMulOperator>(left, right); }
        static CDivOperator* createDivOperator(const CTreeNode* left, const CTreeNode* right) { return createBinaryOperator<CDivOperator>(left, right); }

    private:
        template <class T>
        static T* createBinaryOperator(const CTreeNode* left, const CTreeNode* right)
        {
            T* node = new T();
            node->add_argument(left);
            node->add_argument(right);
            return node;
        }
};

class CParserImpl;

class CParser
{
    public:
        CParser(): m_state(EPS_UNDEFINED) {}
        bool parse(const std::string& expression);

    private:
        CParserImpl* impl();
        CParserImpl* m_impl;

        EParserState m_state;
};

}

#endif
