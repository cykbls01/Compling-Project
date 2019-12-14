#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"


#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {
    class Symbol
    {
    public:

        std::string name;
        std::any value;
        std::string type;
        bool init;
        int index;
        int level;
        std::string func;
        int xiabiao;

    };
    enum INSType{
        NOP=0,
        BIPUSH=1,
        IPUSH=2,
        POP=4,
        POP2=5,
        POPN=6,
        DUP=7,
        DUP2=8,
        LOADC=9,
        LOADA=10,
        NEW=11,
        SNEW=12,
        ILOAD=16,
        DLOAD=17,
        ALOAD=18,
        IALOAD=24,
        DALOAD=25,
        AALOAD=26,
        ISTORE=32,
        DSTORE=33,
        ASTORE=34,
        IASTORE=40,
        DASTORE=41,
        AASTORE=42,
        IADD=48,
        DADD=49,
        ISUB=52,
        DSUB=53,
        IMUL=56,
        DMUL=57,
        IDIV=60,
        DDIV=61,
        INEG=64,
        DENG=65,
        ICMP=68,
        DCMP=69,
        I2D=96,
        D2I=97,
        I2C=98,
        JMP=112,
        JE=113,
        JNE=114,
        JL=115,
        JGE=116,
        JG=117,
        JLE=118,
        CALL=128,
        RET=136,
        IRET=137,
        DRET=138,
        ARET=139,
        IPRINT=160,
        DPRINT=161,
        CPRINT=162,
        SPRINT=163,
        PRINTL=175,
        ISCAN=176,
        DSCAN=177,
        CSCAN=178,
    };
    class Instruct{
    public:
        INSType type;
        int a1;
        int a2;
        int a3;
    };



	class Analyser final {
    public:
	    std::string func=" ";
        std::string type;
        std::string fanhui;
        int level=0;
        int index=0;
        int function_size;
        bool init=false;
        std::map<std::string,std::map<std::string,Symbol>> st;
        std::vector<std::string> ft;
        std::map<std::string,std::string>ftt;
        std::map<std::string,int> fts;
        std::map<std::string,std::vector<std::string>> ins;
        std::map<std::string,std::vector<Instruct>> inst;
        int biaoji;



	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:

		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_uninitialized_vars({}), _vars({}), _consts({}), _nextTokenIndex(0) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;
		bool AddFunc(std::string name);
		bool AddIden(Symbol symbol);
		void AddWhile();
        void AddIf();
        void bianli( std::ostream& output);
        void bianli1( std::ostream& output);
        std::string to_hexa(int n);
        std::string to_number(int c);
        bool check();

        int FindFunc(std::string name);
        void transfer();
        int duihuan(std::string s);
        void split(const std::string& s,std::vector<std::string>& sv);

		// 唯一接口
		std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyse( std::ostream& output,char status);
	private:
		// 所有的递归子程序

		// <程序>
		std::optional<CompilationError> analyseProgram( std::ostream& output,char status);

		// <变量声明>
		std::optional<CompilationError> analyseVariableDeclaration();

		// <表达式>
		std::optional<CompilationError> analyseExpression();
		// <赋值语句>



        std::optional<CompilationError> analyseFunctionDefinition();
        std::optional<CompilationError> analyseInitDeclaratorList();
        std::optional<CompilationError> analyseInitDeclarator();
        std::optional<CompilationError> analyseParameterDeclarationList();
        std::optional<CompilationError> analyseCompoundStatement();
        std::optional<CompilationError> analyseParameterDeclaration();
        std::optional<CompilationError> analyseStatementSeq();
        std::optional<CompilationError> analyseConditionStatement();
        std::optional<CompilationError> analyseLoopStatement();
        std::optional<CompilationError> analyseJumpStatement();
        std::optional<CompilationError> analysePrintStatement();
        std::optional<CompilationError> analysePrintAble();
        std::optional<CompilationError> analyseScanStatement();
        std::optional<CompilationError> analyseAssignmentExpression();
        std::optional<CompilationError> analyseFunctionCall();
        std::optional<CompilationError> analyseStatement();

        std::optional<CompilationError> analyseAdditiveExpression();
        std::optional<CompilationError> analyseMultiplicativeExpression();
        std::optional<CompilationError> analyseUnaryExpression();
        std::optional<CompilationError> analysePrimaryExpression();

        // Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, std::map<std::string, int32_t>&);
		// 添加变量、常量、未初始化的变量
		void addVariable(const Token&);
		void addConstant(const Token&);
		void addUninitializedVariable(const Token&);
		// 是否被声明过
		bool isDeclared(const std::string&);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&);
		// 是否是常量
		bool isConstant(const std::string&);
		// 获得 {变量，常量} 在栈上的偏移
		int32_t getIndex(const std::string&);
	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::vector<Instruction> _instructions;
		std::pair<uint64_t, uint64_t> _current_pos;

		// 为了简单处理，我们直接把符号表耦合在语法分析里
		// 变量                   示例
		// _uninitialized_vars    int a;
		// _vars                  int a=1;
		// _consts                const a=1;
		std::map<std::string, int32_t> _uninitialized_vars;
		std::map<std::string, int32_t> _vars;
		std::map<std::string, int32_t> _consts;
		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;
	};
}