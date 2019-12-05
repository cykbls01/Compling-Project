#include "analyser.h"
//#include "symbol.h"

#include <climits>
#include <3rd_party/fmt/include/fmt/format.h>


namespace miniplc0 {
	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse( std::ostream& output) {
		auto err = analyseProgram(output);
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::=
    //    {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram( std::ostream& output) {

        AddFunc(func);//增加全局函数
        // {<variable-declaration>} 循环体去里面处理

		auto var = analyseVariableDeclaration();//全局变量声明
		if (var.has_value())
			return var;

        // {<function-definition>} 循环体去里面处理
		auto func=analyseFunctionDefinition();//函数声明
		if(func.has_value())
		    return func;
		if(check()==false)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedMain);
		bianli(output);


		return {};
	}


	// <variable-declaration> ::=
    //    [<const-qualifier>]<type-specifier><init-declarator-list>';'
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {

        while (true) {


            // [<const-qualifier>]
            auto next = nextToken();
            if(!next.has_value())
                return {};
            bool biaoji=true;
            type="const";
            if(next.value().GetType()!=TokenType::CONST) {
                biaoji=false;
                unreadToken();
                type="int";
            }//判断是不是const

            // <type-specifier>
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::INT) {
                if(biaoji==false) {
                    unreadToken();
                    return {};
                }
                else//有const，必须包含int
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedInt);
            }
            //函数和标识符区分一下
            next=nextToken();
            next=nextToken();
            if(next.has_value()&&next.value().GetType()==TokenType::LEFT_SMALL_BRACKET)
            {
                if(biaoji==true) {
                    unreadToken();
                    unreadToken();
                    unreadToken();
                    unreadToken();
                }
                else
                {
                    unreadToken();
                    unreadToken();
                    unreadToken();
                }

                return{};
            }
            else
            {
                unreadToken();
                unreadToken();
            }

            // <init-declarator-list>
            auto error=analyseInitDeclaratorList();
            if(error.has_value())
                return error;

        // ';'
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);


        }
		return {};
	}
    // <init-declarator-list> ::=
    //    <init-declarator>{','<init-declarator>}
	std::optional<CompilationError> Analyser::analyseInitDeclaratorList() {
	    // <init-declarator>
	    auto init=analyseInitDeclarator();
	    if(init.has_value())
	        return init;
	    while(true)
        {
	        // ','
	        auto next=nextToken();
	        if(!next.has_value()||next.value().GetType()!=TokenType::DOUHAO)
            {
	            unreadToken();
	            return{};
            }//不是逗号，跳出循环
	        // <init-declarator>

            init=analyseInitDeclarator();
            if(init.has_value())
                return init;
        }
        return {};
	}
	// <init-declarator> ::=
    //    <identifier>[<initializer>]
    // <initializer> ::=
    //    '='<expression>
    std::optional<CompilationError> Analyser::analyseInitDeclarator() {
	    // <identifier>
	    auto next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

        Symbol symbol;
        symbol.name=next.value().GetValueString();


	    // '='
	    next=nextToken();
	    init=true;
        if(!next.has_value()||next.value().GetType()!=TokenType::EQUAL)
        {
            if(type=="const")
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

            unreadToken();
            init=false;

            if(AddIden(symbol)==false)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
            ins[func].push_back("ipush 0");
            return {};
        }
        if(AddIden(symbol)==false)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
        // <expression>
        auto expr=analyseExpression();
        if(expr.has_value())
            return expr;
        return {};
        
	}
	// <function-definition> ::=
    //    <type-specifier><identifier><parameter-clause><compound-statement>
    // <parameter-clause> ::=
    //    '(' [<parameter-declaration-list>] ')'
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {


	    while(true)
        {
	        function_size=0;

	        // <type-specifier>
            auto next=nextToken();
            if(!next.has_value())
                return {};


            if(next.value().GetType()!=TokenType::VOID&&next.value().GetType()!=TokenType::INT)
            {
                return {};
            }
            if(next.value().GetType()==TokenType::VOID)
                fanhui="void";
            else fanhui="int";


            // <identifier>
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
            {
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            }
            if(AddFunc(next.value().GetValueString())==false)
            {
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
            }
            func=next.value().GetValueString();




            // '('
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_SMALL_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
            index++;
            // [<parameter-declaration-list>]
            next=nextToken();
            unreadToken();
            if(next.has_value()&&next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET) {
                auto error = analyseParameterDeclarationList();
                if (error.has_value())
                    return error;
            }
            index--;
            // ')'

            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
            fts[func]=function_size;

            function_size=0;
            // <compound-statement>
            auto error=analyseCompoundStatement();
            if(error.has_value())
                return error;

            ins[func].push_back("ipop");
            if(fanhui=="int")
                ins[func].push_back("ipush 1");
            ins[func].push_back("ret");

            func=" ";


        }
        return {};
	}
    // <parameter-declaration> ::=
    //    [<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseParameterDeclaration(){
	    // [<const-qualifier>]
	    auto next=nextToken();
        type="const";
	    if(!next.has_value()||next.value().GetType()!=TokenType::CONST)
        {
	        unreadToken();
            type="int";

        }

	    // <type-specifier>
	    next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::INT)
        {
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedInt);
        }

        // <identifier>
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
        {
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
        }
        Symbol symbol;
        symbol.name=next.value().GetValueString();
        if(AddIden(symbol)==false)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);





        return {};

	}
	// <parameter-declaration-list> ::=
    //    <parameter-declaration>{','<parameter-declaration>}
    std::optional<CompilationError> Analyser::analyseParameterDeclarationList(){
	    // <parameter-declaration>
	    function_size++;
	    auto error=analyseParameterDeclaration();
        if(error.has_value())
            return error;
        while(true)
        {
            // ','
            auto next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::DOUHAO)
            {
                unreadToken();
                return {};
            }
            // <parameter-declaration>
            error=analyseParameterDeclaration();
            if(error.has_value())
                return error;
            function_size++;
        }
        return {};
	}

    // <compound-statement> ::=
    //    '{' {<variable-declaration>} <statement-seq> '}'
    std::optional<CompilationError> Analyser::analyseCompoundStatement(){
	    // '{'

	    index++;
	    auto next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BIG_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);

	    // {<variable-declaration>}
        next=nextToken();
        unreadToken();
	    if(next.has_value()&&(next.value().GetType()==TokenType::CONST||next.value().GetType()==TokenType::VOID||next.value().GetType()==TokenType::INT))
        {

	        auto error=analyseVariableDeclaration();
            if(error.has_value())
                return error;
        }

	    // <statement-seq>

	    auto error=analyseStatementSeq();
        if(error.has_value())
            return error;
        // '}'
        index--;



        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BIG_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);


        return {};





	}

    // <condition-statement> ::=
    //     'if' '(' <condition> ')' <statement> ['else' <statement>]
    // <condition> ::=
    //     <expression>[<relational-operator><expression>]
    std::optional<CompilationError> Analyser::analyseConditionStatement() {
	    // 'if'
	    auto next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::IF)
        {
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedIf);
        }
	    // '('
	    next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
	    // <expression>
        auto error=analyseExpression();
        if(error.has_value())
            return error;
        // [<relational-operator><expression>]
        next=nextToken();
        unreadToken();
        if(next.has_value()&&(next.value().GetType()==TokenType::LESS||next.value().GetType()==TokenType::LESS_EQUAL||next.value().GetType()==TokenType::GREATER
            ||next.value().GetType()==TokenType::GREATER_EQUAL||next.value().GetType()==TokenType::NO_EQUAL||next.value().GetType()==TokenType::EQUAL_EQUAL))
        {
            std::string aaa=next.value().GetValueString();
            next=nextToken();
            error=analyseExpression();
            if(error.has_value())
                return error;
            ins[func].push_back("icmp");
            ins[func].push_back("ifbegin"+aaa);
        }
        else
        {
            ins[func].push_back("push 0");
            ins[func].push_back("icmp");
            ins[func].push_back("ifbegin!=");
        }
        // ')'
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // <statement>
        error=analyseStatement();
        if(error.has_value())
            return error;


        // 'else'
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::ELSE)
        {
            AddIf();
            unreadToken();
            return {};
        }
        ins[func].push_back("jump?");
        AddIf();
        // <statement>
        error=analyseStatement();
        if(error.has_value())
            return error;
        int i=0;
        for(i=ins[func].size()-1;i>=0;i--)
        {
            if(ins[func][i]=="jump?") {
                ins[func][i] = "jump " + std::to_string(ins[func].size());
            break;

            }
        }

        return {};
	}
	// <loop-statement> ::=
    //    'while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement() {
	    // 'while'
	    auto next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::WHILE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedWhile);
	    // '('
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // <condition>
        biaoji=ins[func].size();

        auto error=analyseExpression();
        if(error.has_value())
            return error;

        // [<relational-operator><expression>]
        next=nextToken();
        unreadToken();
        if(next.has_value()&&(next.value().GetType()==TokenType::LESS||next.value().GetType()==TokenType::LESS_EQUAL||next.value().GetType()==TokenType::GREATER
                              ||next.value().GetType()==TokenType::GREATER_EQUAL||next.value().GetType()==TokenType::NO_EQUAL||next.value().GetType()==TokenType::EQUAL_EQUAL))
        {
            std::string aaa=next.value().GetValueString();
            next=nextToken();

            error=analyseExpression();
            if(error.has_value())
                return error;

            ins[func].push_back("icmp");
            ins[func].push_back("whilebegin"+aaa);
        }
        else
        {
            ins[func].push_back("push 0");
            ins[func].push_back("icmp");
            ins[func].push_back("whilebegin!=");
        }
        // ')'


        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // <statement>
        error=analyseStatement();
        if(error.has_value())
            return error;
        ins[func].push_back("jump "+std::to_string(biaoji));
        AddWhile();
        return {};
	}

    // <jump-statement> ::=
    //    <return-statement>
    // <return-statement> ::=
    //    'return' [<expression>] ';'
    std::optional<CompilationError> Analyser::analyseJumpStatement() {
	    // 'return'
        auto next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RETURN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedReturn);
        next=nextToken();

        // ';'
        if(!next.has_value()||next.value().GetType()==TokenType::SEMICOLON) {
            if(fanhui=="int")
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorReturn);
            ins[func].push_back("ipop");
            ins[func].push_back("ret");
            return {};
        }
        if(fanhui=="void")
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorReturn);
        unreadToken();
        // [<expression>]
        auto error=analyseExpression();
        if(error.has_value())
            return error;
        next=nextToken();
        // ';'
        if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        ins[func].push_back("iret");


        return {};


	}
    // <scan-statement> ::=
    //    'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement() {
	    // 'scan'
        auto next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::SCAN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedScan);

        // '('
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // <identifier>
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
        Symbol symbol;
        symbol.name=next.value().GetValueString();


        if(st[func].find(symbol.name)!=st[func].end())
        {
            if(st[func][symbol.name].type=="const")
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
            ins[func].push_back("loada 0,"+std::to_string(st[func][symbol.name].xiabiao));

        }
        else if(st[" "].find(symbol.name)!=st[" "].end())
        {
            if(st[" "][symbol.name].type=="const")
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
            ins[func].push_back("loada 1,"+std::to_string(st[" "][symbol.name].xiabiao));

        }
        else return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        ins[func].push_back("iscan");
        ins[func].push_back("istore");

        // ')'
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // ';'
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        return {};





	}
    // <print-statement> ::=
    //    'print' '(' [<printable-list>] ')' ';'
    // <printable-list>  ::=
    //    <printable> {',' <printable>}
    // <printable> ::=
    //    <expression>
    std::optional<CompilationError> Analyser::analysePrintStatement() {
	    // 'print'
        auto next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::PRINT)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedPrint);
        // '('
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // [<printable-list>]
        next=nextToken();
        unreadToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
        {

            // <expression>
            auto error=analyseExpression();
            if(error.has_value())
                return error;
            ins[func].push_back("iprint");
            while(true)
            {
                // ','
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::DOUHAO) {
                    unreadToken();
                    break;
                }
                // <expression>
                auto error=analyseExpression();
                if(error.has_value())
                    return error;
                ins[func].push_back("iprint");
            }
        }
        // ')'
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // ';'
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        return {};
	}

    // <assignment-expression> ::=
    //    <identifier><assignment-operator><expression>
    std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
	    // <identifier>
	    auto next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
	    Symbol symbol;
	    symbol.name=next.value().GetValueString();
        int i;
        std::string ttt;
        if(st[func].find(symbol.name)!=st[func].end())
        {
            ttt="feiquanju";

            if(st[func][symbol.name].type=="const")
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
            ins[func].push_back("loada 0,"+std::to_string(st[func][symbol.name].xiabiao));

        }
        else if(st[" "].find(symbol.type)!=st[" "].end())
        {

            ttt="quanju";


            if(st[" "][symbol.name].type=="const")
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
            ins[func].push_back("loada 1,"+std::to_string(st[" "][symbol.name].xiabiao));

        }
        else return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);







	    // <assignment-operator>
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::EQUAL)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedEqual);

        // <expression>
        auto error=analyseExpression();
        if(error.has_value())
            return error;
        ins[func].push_back("istore");
        if(ttt=="feiquanju")
        st[func][symbol.name].init=true;
        else if(ttt=="quanju")
        st[" "][symbol.name].init=true;

        return {};
	}
    // <expression> ::=
    //    <additive-expression>
    std::optional<CompilationError> Analyser::analyseExpression() {
	    // <additive-expression>
	    auto error=analyseAdditiveExpression();
        if(error.has_value())
            return error;
        return{};
	}
	// <primary-expression> ::=
    //     '('<expression>')'
    //    |<identifier>
    //    |<integer-literal>
    //    |<function-call>
    std::optional<CompilationError> Analyser::analysePrimaryExpression() {
	    // '('
	    auto next=nextToken();
	    if(next.has_value()&&next.value().GetType()==TokenType::LEFT_SMALL_BRACKET)
        {
            // <expression>
	        auto error=analyseExpression();
            if(error.has_value())
                return error;
            // ')'
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
            return {};

        }
	    // <identifier>
	    else if(next.has_value()&&next.value().GetType()==TokenType::IDENTIFIER)
        {
            std::string name=next.value().GetValueString();
	        next=nextToken();
            if(next.has_value()&&next.has_value()&&next.value().GetType()==TokenType::LEFT_SMALL_BRACKET)
            {
                unreadToken();
                unreadToken();
                // <function-call>
                auto error=analyseFunctionCall();
                if(error.has_value())
                    return error;
                return {};
            }
            // <identifier>
            else
            {
                unreadToken();
                Symbol symbol;
                int i;
                symbol.name=name;
                if(st[func].find(symbol.name)!=st[func].end())
                {

                    if(st[func][symbol.name].init==false)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
                    ins[func].push_back("loada 0,"+std::to_string(st[func][symbol.name].xiabiao));
                    ins[func].push_back("iload");
                }
                else if(st[" "].find(symbol.name)!=st[" "].end())
                {

                    if(st[func][symbol.name].init== false)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
                    ins[func].push_back("loada 1,"+std::to_string(st[func][symbol.name].xiabiao));
                    ins[func].push_back("iload");
                }
                else return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
                return {};
            }


        }
	    // <integer-literal>
	    else if(next.has_value()&&next.value().GetType()==TokenType::INTEGER) {
	        ins[func].push_back("ipush "+std::to_string(std::any_cast<int>(next.value().GetValue())));
            return {};
        }
	    else
        {

	        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);
        }
	}
    // <unary-expression> ::=
    //    [<unary-operator>]<primary-expression>
    std::optional<CompilationError> Analyser::analyseUnaryExpression() {
	    // [<unary-operator>]
	    auto next=nextToken();
        bool flag=false;
	    if(!next.has_value()||(next.value().GetType()!=TokenType::PLUS&&next.value().GetType()!=TokenType::MINUS))
        {
	        flag=true;
	        unreadToken();
        }

	    // <primary-expression>
	    auto error=analysePrimaryExpression();
        if(error.has_value())
            return error;
        if(flag==false)
        ins[func].push_back("ineg");
        return {};
	}
    // <multiplicative-expression> ::=
    //     <unary-expression>{<multiplicative-operator><unary-expression>}
    std::optional<CompilationError> Analyser::analyseMultiplicativeExpression() {
        // <unary-expression>
        auto error=analyseUnaryExpression();
        if(error.has_value())
            return error;
        while(true)
        {
            // <multiplicative-operator>
            auto next=nextToken();
            TokenType type=next.value().GetType();
            if(!next.has_value()||(next.value().GetType()!=TokenType::MULTIPLICATION&&next.value().GetType()!=TokenType::DIVISION))
            {
                unreadToken();
                return {};
            }
            // <unary-expression>

            error=analyseUnaryExpression();
            if(error.has_value())
                return error;
            if(type==TokenType::MULTIPLICATION)
                ins[func].push_back("imul");
            else
                ins[func].push_back("idiv");


        }
        return{};
	}
    // <additive-expression> ::=
    //     <multiplicative-expression>{<additive-operator><multiplicative-expression>}
    std::optional<CompilationError> Analyser::analyseAdditiveExpression() {
	    // <multiplicative-expression>
        auto error=analyseMultiplicativeExpression();
        if(error.has_value())
            return error;
        while(true)
        {
            // <additive-operator>
            auto next=nextToken();
            TokenType type=next.value().GetType();
            if(!next.has_value()||(next.value().GetType()!=TokenType::PLUS&&next.value().GetType()!=TokenType::MINUS))
            {
                unreadToken();
                return {};
            }
            // <multiplicative-expression>
            error=analyseMultiplicativeExpression();
            if(error.has_value())
                return error;

            if(type==TokenType::PLUS)
                ins[func].push_back("iadd");
            else
                ins[func].push_back("isub");

        }
        return{};
	}

    // <function-call> ::=
    //    <identifier> '(' [<expression-list>] ')'
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
	    // <identifier>
	    auto next=nextToken();
	    if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
	    int aaa=FindFunc(next.value().GetValueString());
        std::string namef=next.value().GetValueString();

	    next=nextToken();
	    // '('
        if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        // ')'
        next=nextToken();
        unreadToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
        {
            // [<expression-list>]
            function_size=1;
            auto error=analyseExpression();
            if(error.has_value())
                return error;
            while(true)
            {
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::DOUHAO)
                {
                    unreadToken();
                    break;
                }
                error=analyseExpression();
                if(error.has_value())
                    return error;
                function_size++;
            }

        }
        if(aaa==0)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        else
            ins[func].push_back("call("+std::to_string(aaa-1)+")");
        next=nextToken();
        // ')'
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_SMALL_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorNeedBracket);
        if(function_size!=fts[namef])
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrorWrongPara);

        return{};




	}



    // <statement-seq> ::=
    //	{<statement>}
    // <statement> ::=
    //     <compound-statement>
    //    |<condition-statement>
    //    |<loop-statement>
    //    |<jump-statement>
    //    |<print-statement>
    //    |<scan-statement>
    //    |<assignment-expression>';'
    //    |<function-call>';'
    //    |';'
    std::optional<CompilationError> Analyser::analyseStatementSeq(){
	    while(true)
        {

	        auto next=nextToken();

            if(!next.has_value())
                return {};

	        switch(next.value().GetType())
            {
                case TokenType ::LEFT_BIG_BRACKET:
                {

                    unreadToken();

                    auto error=analyseCompoundStatement();
                    if(error.has_value())
                        return error;


                    //next=nextToken();

                    //if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BIG_BRACKET)
                        //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrEOF);
                    break;
                }
                case TokenType ::IF:
                {
                    unreadToken();
                    auto error=analyseConditionStatement();
                    if(error.has_value())
                        return error;


                    break;
                }
                case TokenType ::WHILE:
                {
                    unreadToken();
                    auto error=analyseLoopStatement();
                    if(error.has_value())
                        return error;


                    break;
                }
                case TokenType ::RETURN:
                {
                    unreadToken();
                    auto error=analyseJumpStatement();
                    if(error.has_value())
                        return error;


                    break;
                }
                case TokenType ::PRINT:
                {
                    unreadToken();
                    auto error=analysePrintStatement();
                    if(error.has_value())
                        return error;


                    break;
                }
                case TokenType ::SCAN:
                {
                    unreadToken();
                    auto error=analyseScanStatement();
                    if(error.has_value())
                        return error;


                    break;
                }
                case TokenType ::IDENTIFIER:
                {

                    next=nextToken();
                    if(next.has_value()&&next.value().GetType()==TokenType::LEFT_SMALL_BRACKET)
                    {
                        unreadToken();
                        unreadToken();

                        auto error=analyseFunctionCall();
                        if(error.has_value())
                            return error;
                    }
                    else
                    {
                        unreadToken();
                        unreadToken();
                        auto error=analyseAssignmentExpression();
                        if(error.has_value())
                            return error;
                    }
                    next=nextToken();
                    if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);



                    break;
                }



                default:
                    unreadToken();
                    return {};
                    break;




            }



        }


	}
	// <statement> ::=
    //     <compound-statement>
    //    |<condition-statement>
    //    |<loop-statement>
    //    |<jump-statement>
    //    |<print-statement>
    //    |<scan-statement>
    //    |<assignment-expression>';'
    //    |<function-call>';'
    //    |';'
	std::optional<CompilationError> Analyser::analyseStatement() {
        auto next=nextToken();
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidInput);

        switch(next.value().GetType())
        {
            case TokenType ::LEFT_BIG_BRACKET:
            {
                unreadToken();
                auto error=analyseCompoundStatement();
                if(error.has_value())
                    return error;

                break;
            }
            case TokenType ::IF:
            {
                unreadToken();
                auto error=analyseConditionStatement();
                if(error.has_value())
                    return error;


                break;
            }
            case TokenType ::WHILE:
            {
                unreadToken();
                auto error=analyseLoopStatement();
                if(error.has_value())
                    return error;


                break;
            }
            case TokenType ::RETURN:
            {
                unreadToken();
                auto error=analyseJumpStatement();
                if(error.has_value())
                    return error;


                break;
            }
            case TokenType ::PRINT:
            {
                unreadToken();
                auto error=analysePrintStatement();
                if(error.has_value())
                    return error;


                break;
            }
            case TokenType ::SCAN:
            {
                unreadToken();
                auto error=analyseScanStatement();
                if(error.has_value())
                    return error;


                break;
            }
            case TokenType ::IDENTIFIER:
            {
                next=nextToken();
                if(next.has_value()&&next.value().GetType()==TokenType::LEFT_SMALL_BRACKET)
                {
                    unreadToken();
                    unreadToken();
                    auto error=analyseFunctionCall();
                    if(error.has_value())
                        return error;
                }
                else
                {
                    unreadToken();
                    unreadToken();
                    auto error=analyseAssignmentExpression();
                    if(error.has_value())
                        return error;
                }
                break;
            }
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrEOF);
                break;
        }
	}















	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

    bool Analyser::AddFunc(std::string name) {
	    if(st.find(name)!=st.end())
        {

	        return false;


        }

	    else
        {

	        std::map<std::string,Symbol> vt;
	        st[name]=vt;
	        ft.push_back(name);

	        return true;
        }
	}

	bool Analyser::AddIden(miniplc0::Symbol symbol) {

	    int i;
	    if(st[func].find(symbol.name)!=st[func].end())
	        return false;
	    symbol.level=st[func].size();
	    symbol.index=index;
	    symbol.func=func;
	    symbol.init=init;
	    symbol.type=type;
	    symbol.xiabiao=st[func].size();
	    st[func][symbol.name]=symbol;
	    return true;

	}



    int Analyser::FindFunc(std::string name) {
	    int i;
        for(i=0;i<ft.size();i++)
        {
            if(ft[i]==name)
            {
                return i;
            }
        }
        return 0;




	}

	void Analyser::AddWhile() {
	    int i;
	    for(i=ins[func].size();i>=0;i--)
        {
	        if(ins[func][i]=="whilebegin!=")
            {
                ins[func][i]="je "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="whilebegin==")
            {
                ins[func][i]="jne "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="whilebegin>=")
            {
                ins[func][i]="jl "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="whilebegin>")
            {
                ins[func][i]="jle "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="whilebegin<=")
            {
                ins[func][i]="jg "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="whilebegin<")
            {
                ins[func][i]="jge "+std::to_string(ins[func].size());
                return;
            }


        }






	}

    void Analyser::AddIf() {
        int i;
        for(i=ins[func].size();i>=0;i--)
        {
            if(ins[func][i]=="ifbegin!=")
            {
                ins[func][i]="je "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="ifbegin==")
            {
                ins[func][i]="jne "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="ifbegin>=")
            {
                ins[func][i]="jl "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="ifbegin>")
            {
                ins[func][i]="jle "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="ifbegin<=")
            {
                ins[func][i]="jg "+std::to_string(ins[func].size());
                return;
            }
            if(ins[func][i]=="ifbegin<")
            {
                ins[func][i]="jge "+std::to_string(ins[func].size());
                return;
            }


        }






    }

    void Analyser::bianli( std::ostream& output) {

        output << fmt::format(".constants:\n");
        for(int i=1;i<ft.size();i++)
        {
            std::string aaa=std::to_string(i-1)+" S "+"\""+ft[i]+"\"";
            output <<fmt::format("{}\n",aaa);


        }

        output << fmt::format(".start:\n");
        for(int i=0;i<ins[" "].size();i++)
        {
            std::string aaa=std::to_string(i)+"    "+ins[" "][i];
            output <<fmt::format("{}\n",aaa);


        }

        output << fmt::format(".functions:\n");
        for(int i=1;i<ft.size();i++)
        {
            std::string aaa=std::to_string(i-1)+" "+std::to_string(i-1)+" "+std::to_string(fts[ft[i]])+" 1";
            output <<fmt::format("{}\n",aaa);

        }

        for(int i=1;i<ft.size();i++)
        {
            std::string aaa=".F"+std::to_string(i-1);
            output <<fmt::format("{}\n",aaa);
            for(int j=0;j<ins[ft[i]].size();j++)
            {
                std::string bbb=std::to_string(j)+"    "+ins[ft[i]][j];
                output <<fmt::format("{}\n",bbb);
            }

        }






	}

	bool Analyser::check() {
	    int i=0;
	    for(i=0;i<ft.size();i++)
        {
	        if(ft[i]=="main")
	            return true;


        }
	    return false;


	}








}