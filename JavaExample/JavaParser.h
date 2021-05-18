//
// This is the grammar specification from the Final Draft of the generic spec.
//
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 IBM Corporation.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v1.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v10.html
//
//Contributors:
//    Philippe Charles (pcharles@us.ibm.com) - initial API and implementation

////////////////////////////////////////////////////////////////////////////////


    //#line 161 "btParserTemplateF.gi

#pragma once

#include <iostream>

#include "BacktrackingParser.h"
#include "DeterministicParser.h"
#include "diagnose.h"
#include "ErrorToken.h"
#include "Exception.h"
#include "IAbstractArrayList.h"
#include "IAst.h"
#include "IAstVisitor.h"
#include "ILexStream.h"
#include "JavaParsersym.h"
#include "JavaParserprs.h"
#include "Object.h"
#include "ParseTable.h"
#include "PrsStream.h"
#include "RuleAction.h"
#include "IcuUtil.h"

    //#line 17 "GJavaParser.g



    //#line 186 "btParserTemplateF.gi

struct JavaParser :public Object, public RuleAction
{
    PrsStream* prsStream = nullptr;

    bool unimplementedSymbolsWarning = false;

    inline static ParseTable* prsTable = new JavaParserprs();
    ParseTable* getParseTable() { return prsTable; }

    BacktrackingParser* btParser = nullptr;
    BacktrackingParser* getParser() { return btParser; }

    void setResult(Object* object) { btParser->setSym1(object); }
    Object* getRhsSym(int i) { return btParser->getSym(i); }

    int getRhsTokenIndex(int i) { return btParser->getToken(i); }
    IToken* getRhsIToken(int i) { return prsStream->getIToken(getRhsTokenIndex(i)); }

    int getRhsFirstTokenIndex(int i) { return btParser->getFirstToken(i); }
    IToken* getRhsFirstIToken(int i) { return prsStream->getIToken(getRhsFirstTokenIndex(i)); }

    int getRhsLastTokenIndex(int i) { return btParser->getLastToken(i); }
    IToken* getRhsLastIToken(int i) { return prsStream->getIToken(getRhsLastTokenIndex(i)); }

    int getLeftSpan() { return btParser->getFirstToken(); }
    IToken* getLeftIToken() { return prsStream->getIToken(getLeftSpan()); }

    int getRightSpan() { return btParser->getLastToken(); }
    IToken* getRightIToken() { return prsStream->getIToken(getRightSpan()); }

    int getRhsErrorTokenIndex(int i)
    {
        int index = btParser->getToken(i);
        IToken* err = prsStream->getIToken(index);
        return (dynamic_cast<ErrorToken*>(err) ? index : 0);
    }
    ErrorToken* getRhsErrorIToken(int i)
    {
        int index = btParser->getToken(i);
        IToken* err = prsStream->getIToken(index);
        return (ErrorToken*)(dynamic_cast<ErrorToken*>(err) ? err : nullptr);
    }

    void reset(ILexStream* lexStream)
    {
        prsStream = new PrsStream(lexStream);
        btParser->reset(prsStream);

        try
        {
            prsStream->remapTerminalSymbols(orderedTerminalSymbols(), prsTable->getEoftSymbol());
        }
        catch (NullExportedSymbolsException& e) {
        }
        catch (NullTerminalSymbolsException& e) {
        }
        catch (UnimplementedTerminalsException& e)
        {
            if (unimplementedSymbolsWarning) {
                auto unimplemented_symbols = e.getSymbols();
                std::cout << "The Lexer will not scan the following token(s):" << std::endl;
                for (int i = 0; i < unimplemented_symbols.size(); i++)
                {
                    auto id = unimplemented_symbols.at(i);
                    std::wcout << L"    " << JavaParsersym::orderedTerminalSymbols[id] << std::endl;
                }
                std::cout << std::endl;
            }
        }
        catch (UndefinedEofSymbolException& e)
        {
            std::stringex str = "The Lexer does not implement the Eof symbol ";
            str += IcuUtil::ws2s(JavaParsersym::orderedTerminalSymbols[prsTable->getEoftSymbol()]);
            throw  UndefinedEofSymbolException(str);
        }
    }

    JavaParser(ILexStream* lexStream = nullptr)
    {
        try
        {
            btParser = new BacktrackingParser(prsStream, prsTable, this);
        }
        catch (NotBacktrackParseTableException& e)
        {
            throw (NotBacktrackParseTableException
            ("Regenerate JavaParserprs.java with -BACKTRACK option"));
        }
        catch (BadParseSymFileException& e)
        {
            throw (BadParseSymFileException("Bad Parser Symbol File -- JavaParsersym::java"));
        }

        if (lexStream)
        {
            reset(lexStream);
        }
    }



    int numTokenKinds() { return JavaParsersym::numTokenKinds; }
    std::vector<std::wstring> orderedTerminalSymbols() {
        return JavaParsersym::orderedTerminalSymbols;
    }
    std::wstring getTokenKindName(int kind) { return JavaParsersym::orderedTerminalSymbols[kind]; }
    int getEOFTokenKind() { return prsTable->getEoftSymbol(); }
    IPrsStream* getIPrsStream() { return prsStream; }

    /**
     * @deprecated replaced by {@link #getIPrsStream()}
     *
     */
    PrsStream* getPrsStream() { return prsStream; }

    /**
     * @deprecated replaced by {@link #getIPrsStream()}
     *
     */
    PrsStream* getParseStream() { return prsStream; }
    struct Ast;
    Ast* parser()
    {
        return parser(nullptr, 0);
    }

    Ast* parser(Monitor* monitor)
    {
        return parser(monitor, 0);
    }

    Ast* parser(int error_repair_count)
    {
        return parser(nullptr, error_repair_count);
    }

    Ast* parser(Monitor* monitor, int error_repair_count)
    {
        btParser->setMonitor(monitor);

        try
        {
            return (Ast*)btParser->fuzzyParse(error_repair_count);
        }
        catch (BadParseException& e)
        {
            prsStream->reset(e.error_token); // point to error token

            std::shared_ptr< DiagnoseParser> diagnoseParser = std::make_shared<DiagnoseParser>(prsStream, prsTable);
            diagnoseParser->diagnose(e.error_token);
        }

        return nullptr;
    }

    //
    // Additional entry points, if any
    //

    Ast* parseClassBodyDeclarationsopt()
    {
        return parseClassBodyDeclarationsopt(nullptr, 0);
    }

    Ast* parseClassBodyDeclarationsopt(Monitor* monitor)
    {
        return parseClassBodyDeclarationsopt(monitor, 0);
    }

    Ast* parseClassBodyDeclarationsopt(int error_repair_count)
    {
        return parseClassBodyDeclarationsopt(nullptr, error_repair_count);
    }

    Ast* parseClassBodyDeclarationsopt(Monitor* monitor, int error_repair_count)
    {
        btParser->setMonitor(monitor);

        try
        {
            return (Ast*)btParser->fuzzyParseEntry(JavaParsersym::TK_ClassBodyDeclarationsoptMarker, error_repair_count);
        }
        catch (BadParseException& e)
        {
            prsStream->reset(e.error_token); // point to error token

            std::shared_ptr< DiagnoseParser> diagnoseParser = std::make_shared<DiagnoseParser>(prsStream, prsTable);
            diagnoseParser->diagnoseEntry(JavaParsersym::TK_ClassBodyDeclarationsoptMarker, e.error_token);
        }

        return nullptr;
    }

    Ast* parseLPGUserAction()
    {
        return parseLPGUserAction(nullptr, 0);
    }

    Ast* parseLPGUserAction(Monitor* monitor)
    {
        return parseLPGUserAction(monitor, 0);
    }

    Ast* parseLPGUserAction(int error_repair_count)
    {
        return parseLPGUserAction(nullptr, error_repair_count);
    }

    Ast* parseLPGUserAction(Monitor* monitor, int error_repair_count)
    {
        btParser->setMonitor(monitor);

        try
        {
            return (Ast*)btParser->fuzzyParseEntry(JavaParsersym::TK_LPGUserActionMarker, error_repair_count);
        }
        catch (BadParseException& e)
        {
            prsStream->reset(e.error_token); // point to error token

            std::shared_ptr< DiagnoseParser> diagnoseParser = std::make_shared<DiagnoseParser>(prsStream, prsTable);
            diagnoseParser->diagnoseEntry(JavaParsersym::TK_LPGUserActionMarker, e.error_token);
        }

        return nullptr;
    }

    struct Ast;
    struct AbstractAstList;
    struct AstToken;
    struct identifier;
    struct PrimitiveType;
    struct ClassType;
    struct InterfaceType;
    struct TypeName;
    struct ArrayType;
    struct TypeParameter;
    struct TypeBound;
    struct AdditionalBoundList;
    struct AdditionalBound;
    struct TypeArguments;
    struct ActualTypeArgumentList;
    struct Wildcard;
    struct PackageName;
    struct ExpressionName;
    struct MethodName;
    struct PackageOrTypeName;
    struct AmbiguousName;
    struct CompilationUnit;
    struct ImportDeclarations;
    struct TypeDeclarations;
    struct PackageDeclaration;
    struct SingleTypeImportDeclaration;
    struct TypeImportOnDemandDeclaration;
    struct SingleStaticImportDeclaration;
    struct StaticImportOnDemandDeclaration;
    struct TypeDeclaration;
    struct NormalClassDeclaration;
    struct ClassModifiers;
    struct TypeParameters;
    struct TypeParameterList;
    struct Super;
    struct Interfaces;
    struct InterfaceTypeList;
    struct ClassBody;
    struct ClassBodyDeclarations;
    struct ClassMemberDeclaration;
    struct FieldDeclaration;
    struct VariableDeclarators;
    struct VariableDeclarator;
    struct VariableDeclaratorId;
    struct FieldModifiers;
    struct MethodDeclaration;
    struct MethodHeader;
    struct ResultType;
    struct FormalParameterList;
    struct FormalParameters;
    struct FormalParameter;
    struct VariableModifiers;
    struct VariableModifier;
    struct LastFormalParameter;
    struct MethodModifiers;
    struct Throws;
    struct ExceptionTypeList;
    struct MethodBody;
    struct StaticInitializer;
    struct ConstructorDeclaration;
    struct ConstructorDeclarator;
    struct ConstructorModifiers;
    struct ConstructorBody;
    struct EnumDeclaration;
    struct EnumBody;
    struct EnumConstants;
    struct EnumConstant;
    struct Arguments;
    struct EnumBodyDeclarations;
    struct NormalInterfaceDeclaration;
    struct InterfaceModifiers;
    struct InterfaceBody;
    struct InterfaceMemberDeclarations;
    struct InterfaceMemberDeclaration;
    struct ConstantDeclaration;
    struct ConstantModifiers;
    struct AbstractMethodDeclaration;
    struct AbstractMethodModifiers;
    struct AnnotationTypeDeclaration;
    struct AnnotationTypeBody;
    struct AnnotationTypeElementDeclarations;
    struct DefaultValue;
    struct Annotations;
    struct NormalAnnotation;
    struct ElementValuePairs;
    struct ElementValuePair;
    struct ElementValueArrayInitializer;
    struct ElementValues;
    struct MarkerAnnotation;
    struct SingleElementAnnotation;
    struct ArrayInitializer;
    struct VariableInitializers;
    struct Block;
    struct BlockStatements;
    struct LocalVariableDeclarationStatement;
    struct LocalVariableDeclaration;
    struct IfThenStatement;
    struct IfThenElseStatement;
    struct IfThenElseStatementNoShortIf;
    struct EmptyStatement;
    struct LabeledStatement;
    struct LabeledStatementNoShortIf;
    struct ExpressionStatement;
    struct SwitchStatement;
    struct SwitchBlock;
    struct SwitchBlockStatementGroups;
    struct SwitchBlockStatementGroup;
    struct SwitchLabels;
    struct WhileStatement;
    struct WhileStatementNoShortIf;
    struct DoStatement;
    struct BasicForStatement;
    struct ForStatementNoShortIf;
    struct StatementExpressionList;
    struct EnhancedForStatement;
    struct BreakStatement;
    struct ContinueStatement;
    struct ReturnStatement;
    struct ThrowStatement;
    struct SynchronizedStatement;
    struct Catches;
    struct CatchClause;
    struct Finally;
    struct ArgumentList;
    struct DimExprs;
    struct DimExpr;
    struct PostIncrementExpression;
    struct PostDecrementExpression;
    struct PreIncrementExpression;
    struct PreDecrementExpression;
    struct AndExpression;
    struct ExclusiveOrExpression;
    struct InclusiveOrExpression;
    struct ConditionalAndExpression;
    struct ConditionalOrExpression;
    struct ConditionalExpression;
    struct Assignment;
    struct Commaopt;
    struct Ellipsisopt;
    struct LPGUserAction0;
    struct LPGUserAction1;
    struct LPGUserAction2;
    struct LPGUserAction3;
    struct LPGUserAction4;
    struct IntegralType0;
    struct IntegralType1;
    struct IntegralType2;
    struct IntegralType3;
    struct IntegralType4;
    struct FloatingPointType0;
    struct FloatingPointType1;
    struct WildcardBounds0;
    struct WildcardBounds1;
    struct ClassModifier0;
    struct ClassModifier1;
    struct ClassModifier2;
    struct ClassModifier3;
    struct ClassModifier4;
    struct ClassModifier5;
    struct ClassModifier6;
    struct FieldModifier0;
    struct FieldModifier1;
    struct FieldModifier2;
    struct FieldModifier3;
    struct FieldModifier4;
    struct FieldModifier5;
    struct FieldModifier6;
    struct MethodDeclarator0;
    struct MethodDeclarator1;
    struct MethodModifier0;
    struct MethodModifier1;
    struct MethodModifier2;
    struct MethodModifier3;
    struct MethodModifier4;
    struct MethodModifier5;
    struct MethodModifier6;
    struct MethodModifier7;
    struct MethodModifier8;
    struct ConstructorModifier0;
    struct ConstructorModifier1;
    struct ConstructorModifier2;
    struct ExplicitConstructorInvocation0;
    struct ExplicitConstructorInvocation1;
    struct ExplicitConstructorInvocation2;
    struct InterfaceModifier0;
    struct InterfaceModifier1;
    struct InterfaceModifier2;
    struct InterfaceModifier3;
    struct InterfaceModifier4;
    struct InterfaceModifier5;
    struct ExtendsInterfaces0;
    struct ExtendsInterfaces1;
    struct ConstantModifier0;
    struct ConstantModifier1;
    struct ConstantModifier2;
    struct AbstractMethodModifier0;
    struct AbstractMethodModifier1;
    struct AnnotationTypeElementDeclaration0;
    struct AnnotationTypeElementDeclaration1;
    struct AssertStatement0;
    struct AssertStatement1;
    struct SwitchLabel0;
    struct SwitchLabel1;
    struct SwitchLabel2;
    struct TryStatement0;
    struct TryStatement1;
    struct PrimaryNoNewArray0;
    struct PrimaryNoNewArray1;
    struct PrimaryNoNewArray2;
    struct PrimaryNoNewArray3;
    struct PrimaryNoNewArray4;
    struct Literal0;
    struct Literal1;
    struct Literal2;
    struct Literal3;
    struct Literal4;
    struct Literal5;
    struct Literal6;
    struct BooleanLiteral0;
    struct BooleanLiteral1;
    struct ClassInstanceCreationExpression0;
    struct ClassInstanceCreationExpression1;
    struct ArrayCreationExpression0;
    struct ArrayCreationExpression1;
    struct ArrayCreationExpression2;
    struct ArrayCreationExpression3;
    struct Dims0;
    struct Dims1;
    struct FieldAccess0;
    struct FieldAccess1;
    struct FieldAccess2;
    struct MethodInvocation0;
    struct MethodInvocation1;
    struct MethodInvocation2;
    struct MethodInvocation3;
    struct MethodInvocation4;
    struct ArrayAccess0;
    struct ArrayAccess1;
    struct UnaryExpression0;
    struct UnaryExpression1;
    struct UnaryExpressionNotPlusMinus0;
    struct UnaryExpressionNotPlusMinus1;
    struct CastExpression0;
    struct CastExpression1;
    struct MultiplicativeExpression0;
    struct MultiplicativeExpression1;
    struct MultiplicativeExpression2;
    struct AdditiveExpression0;
    struct AdditiveExpression1;
    struct ShiftExpression0;
    struct ShiftExpression1;
    struct ShiftExpression2;
    struct RelationalExpression0;
    struct RelationalExpression1;
    struct RelationalExpression2;
    struct RelationalExpression3;
    struct RelationalExpression4;
    struct EqualityExpression0;
    struct EqualityExpression1;
    struct AssignmentOperator0;
    struct AssignmentOperator1;
    struct AssignmentOperator2;
    struct AssignmentOperator3;
    struct AssignmentOperator4;
    struct AssignmentOperator5;
    struct AssignmentOperator6;
    struct AssignmentOperator7;
    struct AssignmentOperator8;
    struct AssignmentOperator9;
    struct AssignmentOperator10;
    struct AssignmentOperator11;
    struct Visitor;
    struct AbstractVisitor;
    struct pool_holder {
        Tuple<IAst*> data;
        ~pool_holder() {
            for (int i = 0; i < data.size(); ++i) {
                delete data[i];
            }
        }
    };
    pool_holder ast_pool;
    struct Ast :public IAst
    {
        IAst* getNextAst() { return nullptr; }
        IToken* leftIToken = nullptr;
        IToken* rightIToken = nullptr;
        IAst* parent = nullptr;
        void setParent(IAst* parent) { this->parent = parent; }
        IAst* getParent() { return parent; }

        IToken* getLeftIToken() { return leftIToken; }
        IToken* getRightIToken() { return rightIToken; }
        std::vector<IToken*> getPrecedingAdjuncts() { return leftIToken->getPrecedingAdjuncts(); }
        std::vector<IToken*> getFollowingAdjuncts() { return rightIToken->getFollowingAdjuncts(); }

        std::wstring toString()
        {
            return leftIToken->getILexStream()->toString(leftIToken->getStartOffset(), rightIToken->getEndOffset());
        }

        Ast(IToken* token) { this->leftIToken = this->rightIToken = token; }
        Ast(IToken* leftIToken, IToken* rightIToken)
        {
            this->leftIToken = leftIToken;
            this->rightIToken = rightIToken;
        }

        void initialize() {}

        /**
         * A list of all children of this node, excluding the nullptr ones.
         */
        std::vector<IAst*> getChildren()
        {
            std::vector<IAst*> list = getAllChildren();
            int k = -1;
            for (int i = 0; i < list.size(); i++)
            {
                IAst* element = list[i];
                if (element != nullptr)
                {
                    if (++k != i)
                        list[k] = (element);
                }
            }
            for (int i = list.size() - 1; i > k; i--) // remove extraneous elements
                list.erase(list.begin() + i);
            return list;
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        virtual std::vector<IAst*> getAllChildren() = 0;

        virtual void accept(IAstVisitor* v) = 0;
    };

    struct AbstractAstList :public Ast, public IAbstractArrayList<Ast*>
    {
        bool leftRecursive;
        int size() { return list.size(); }
        std::vector<Ast*> getList() { return list; }
        Ast* getElementAt(int i) { return (Ast*)list.at(leftRecursive ? i : list.size() - 1 - i); }
        std::vector<IAst*> getArrayList()
        {
            if (!leftRecursive) // reverse the list 
            {
                for (int i = 0, n = list.size() - 1; i < n; i++, n--)
                {
                    auto ith = list[i];
                    auto nth = list[n];
                    list[i] = (nth);
                    list[n] = (ith);
                }
                leftRecursive = true;
            }
            return  std::vector<IAst*>(list.begin(), list.end());
        }
        /**
         * @deprecated replaced by {@link #addElement()}
         *
         */
        bool add(Ast* element)
        {
            addElement(element);
            return true;
        }

        void addElement(Ast* element)
        {
            list.push_back(element);
            if (leftRecursive)
                rightIToken = element->getRightIToken();
            else leftIToken = element->getLeftIToken();
        }

        AbstractAstList(IToken* leftIToken, IToken* rightIToken, bool leftRecursive) :Ast(leftIToken, rightIToken)
        {
            this->leftRecursive = leftRecursive;
        }

        AbstractAstList(Ast* element, bool leftRecursive) :Ast(element->getLeftIToken(), element->getRightIToken())
        {
            this->leftRecursive = leftRecursive;
            list.push_back(element);
        }

        /**
         * Make a copy of the list and return it. Note that we obtain the local list by
         * invoking getArrayList so as to make sure that the list we return is in proper order.
         */
        std::vector<IAst*> getAllChildren()
        {
            auto list_ = getArrayList();
            return   std::vector<IAst*>(list_.begin(), list_.end());
        }

    };

    struct AstToken :public  Ast
    {
        AstToken(IToken* token) :Ast(token) { }
        IToken* getIToken() { return leftIToken; }
        std::wstring toString() { return leftIToken->toString(); }

        /**
         * A token class has no children. So, we return the empty list.
         */
        std::vector<IAst*> getAllChildren() { return {}; }


        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 3:  identifier ::= IDENTIFIER
     *</b>
     */
    struct identifier :public AstToken
    {
        JavaParser* environment;
        JavaParser* getEnvironment() { return environment; }

        IToken* getIDENTIFIER() { return leftIToken; }

        identifier(JavaParser* environment, IToken* token) :AstToken(token)
        {
            this->environment = environment;
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }

        //#line 185 "GJavaParser.g

        void initialize()
        {
            if (getIDENTIFIER()->getKind() != JavaParserprs::TK_IDENTIFIER)
                std::wcout << L"Turning keyword " << getIDENTIFIER()->toString() << L" into an identifier" << std::endl;
        }
    };

    /**
     *<em>
     *<li>Rule 12:  PrimitiveType ::= NumericType
     *</em>
     *<p>
     *<b>
     *<li>Rule 13:  PrimitiveType ::= boolean
     *</b>
     */
    struct PrimitiveType :public AstToken
    {
        IToken* getboolean() { return leftIToken; }

        PrimitiveType(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 27:  ClassType ::= TypeName TypeArgumentsopt
     *</b>
     */
    struct ClassType :public Ast
    {
        IAst* lpg_TypeName;
        IAst* lpg_TypeArgumentsopt;

        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }

        ClassType(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeName,
            IAst* lpg_TypeArgumentsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeName);
            list.push_back(lpg_TypeArgumentsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeName->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 28:  InterfaceType ::= TypeName TypeArgumentsopt
     *</b>
     */
    struct InterfaceType :public Ast
    {
        IAst* lpg_TypeName;
        IAst* lpg_TypeArgumentsopt;

        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }

        InterfaceType(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeName,
            IAst* lpg_TypeArgumentsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeName);
            list.push_back(lpg_TypeArgumentsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeName->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 29:  TypeName ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 30:  TypeName ::= TypeName . identifier
     *</b>
     */
    struct TypeName :public Ast
    {
        IAst* lpg_TypeName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        TypeName(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeName,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 33:  ArrayType ::= Type [ ]
     *</b>
     */
    struct ArrayType :public Ast
    {
        IAst* lpg_Type;
        IAst* lpg_LBRACKET;
        IAst* lpg_RBRACKET;

        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        ArrayType(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Type,
            IAst* lpg_LBRACKET,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Type);
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Type->accept(v);
                lpg_LBRACKET->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 34:  TypeParameter ::= TypeVariable TypeBoundopt
     *</b>
     */
    struct TypeParameter :public Ast
    {
        IAst* lpg_TypeVariable;
        IAst* lpg_TypeBoundopt;

        IAst* getTypeVariable() { return lpg_TypeVariable; };
        void setTypeVariable(IAst* lpg_TypeVariable) { this->lpg_TypeVariable = lpg_TypeVariable; }
        /**
         * The value returned by <b>getTypeBoundopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeBoundopt() { return lpg_TypeBoundopt; };
        void setTypeBoundopt(IAst* lpg_TypeBoundopt) { this->lpg_TypeBoundopt = lpg_TypeBoundopt; }

        TypeParameter(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeVariable,
            IAst* lpg_TypeBoundopt) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeVariable = lpg_TypeVariable;
            ((Ast*)lpg_TypeVariable)->setParent(this);
            this->lpg_TypeBoundopt = lpg_TypeBoundopt;
            if (lpg_TypeBoundopt != nullptr) ((Ast*)lpg_TypeBoundopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeVariable);
            list.push_back(lpg_TypeBoundopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeVariable->accept(v);
                if (lpg_TypeBoundopt != nullptr) lpg_TypeBoundopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 35:  TypeBound ::= extends ClassOrInterfaceType AdditionalBoundListopt
     *</b>
     */
    struct TypeBound :public Ast
    {
        IAst* lpg_extends;
        IAst* lpg_ClassOrInterfaceType;
        IAst* lpg_AdditionalBoundListopt;

        IAst* getextends() { return lpg_extends; };
        void setextends(IAst* lpg_extends) { this->lpg_extends = lpg_extends; }
        IAst* getClassOrInterfaceType() { return lpg_ClassOrInterfaceType; };
        void setClassOrInterfaceType(IAst* lpg_ClassOrInterfaceType) { this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType; }
        /**
         * The value returned by <b>getAdditionalBoundListopt</b> may be <b>nullptr</b>
         */
        IAst* getAdditionalBoundListopt() { return lpg_AdditionalBoundListopt; };
        void setAdditionalBoundListopt(IAst* lpg_AdditionalBoundListopt) { this->lpg_AdditionalBoundListopt = lpg_AdditionalBoundListopt; }

        TypeBound(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_extends,
            IAst* lpg_ClassOrInterfaceType,
            IAst* lpg_AdditionalBoundListopt) :Ast(leftIToken, rightIToken) {
            this->lpg_extends = lpg_extends;
            ((Ast*)lpg_extends)->setParent(this);
            this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType;
            ((Ast*)lpg_ClassOrInterfaceType)->setParent(this);
            this->lpg_AdditionalBoundListopt = lpg_AdditionalBoundListopt;
            if (lpg_AdditionalBoundListopt != nullptr) ((Ast*)lpg_AdditionalBoundListopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_extends);
            list.push_back(lpg_ClassOrInterfaceType);
            list.push_back(lpg_AdditionalBoundListopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_extends->accept(v);
                lpg_ClassOrInterfaceType->accept(v);
                if (lpg_AdditionalBoundListopt != nullptr) lpg_AdditionalBoundListopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 36:  AdditionalBoundList ::= AdditionalBound
     *</em>
     *<p>
     *<b>
     *<li>Rule 37:  AdditionalBoundList ::= AdditionalBoundList AdditionalBound
     *</b>
     */
    struct AdditionalBoundList :public Ast
    {
        IAst* lpg_AdditionalBoundList;
        IAst* lpg_AdditionalBound;

        IAst* getAdditionalBoundList() { return lpg_AdditionalBoundList; };
        void setAdditionalBoundList(IAst* lpg_AdditionalBoundList) { this->lpg_AdditionalBoundList = lpg_AdditionalBoundList; }
        IAst* getAdditionalBound() { return lpg_AdditionalBound; };
        void setAdditionalBound(IAst* lpg_AdditionalBound) { this->lpg_AdditionalBound = lpg_AdditionalBound; }

        AdditionalBoundList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AdditionalBoundList,
            IAst* lpg_AdditionalBound) :Ast(leftIToken, rightIToken) {
            this->lpg_AdditionalBoundList = lpg_AdditionalBoundList;
            ((Ast*)lpg_AdditionalBoundList)->setParent(this);
            this->lpg_AdditionalBound = lpg_AdditionalBound;
            ((Ast*)lpg_AdditionalBound)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AdditionalBoundList);
            list.push_back(lpg_AdditionalBound);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AdditionalBoundList->accept(v);
                lpg_AdditionalBound->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 38:  AdditionalBound ::= & InterfaceType
     *</b>
     */
    struct AdditionalBound :public Ast
    {
        IAst* lpg_AND;
        IAst* lpg_InterfaceType;

        IAst* getAND() { return lpg_AND; };
        void setAND(IAst* lpg_AND) { this->lpg_AND = lpg_AND; }
        IAst* getInterfaceType() { return lpg_InterfaceType; };
        void setInterfaceType(IAst* lpg_InterfaceType) { this->lpg_InterfaceType = lpg_InterfaceType; }

        AdditionalBound(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AND,
            IAst* lpg_InterfaceType) :Ast(leftIToken, rightIToken) {
            this->lpg_AND = lpg_AND;
            ((Ast*)lpg_AND)->setParent(this);
            this->lpg_InterfaceType = lpg_InterfaceType;
            ((Ast*)lpg_InterfaceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AND);
            list.push_back(lpg_InterfaceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AND->accept(v);
                lpg_InterfaceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 39:  TypeArguments ::= < ActualTypeArgumentList >
     *</b>
     */
    struct TypeArguments :public Ast
    {
        IAst* lpg_LESS;
        IAst* lpg_ActualTypeArgumentList;
        IAst* lpg_GREATER;

        IAst* getLESS() { return lpg_LESS; };
        void setLESS(IAst* lpg_LESS) { this->lpg_LESS = lpg_LESS; }
        IAst* getActualTypeArgumentList() { return lpg_ActualTypeArgumentList; };
        void setActualTypeArgumentList(IAst* lpg_ActualTypeArgumentList) { this->lpg_ActualTypeArgumentList = lpg_ActualTypeArgumentList; }
        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }

        TypeArguments(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LESS,
            IAst* lpg_ActualTypeArgumentList,
            IAst* lpg_GREATER) :Ast(leftIToken, rightIToken) {
            this->lpg_LESS = lpg_LESS;
            ((Ast*)lpg_LESS)->setParent(this);
            this->lpg_ActualTypeArgumentList = lpg_ActualTypeArgumentList;
            ((Ast*)lpg_ActualTypeArgumentList)->setParent(this);
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LESS);
            list.push_back(lpg_ActualTypeArgumentList);
            list.push_back(lpg_GREATER);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LESS->accept(v);
                lpg_ActualTypeArgumentList->accept(v);
                lpg_GREATER->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 40:  ActualTypeArgumentList ::= ActualTypeArgument
     *</em>
     *<p>
     *<b>
     *<li>Rule 41:  ActualTypeArgumentList ::= ActualTypeArgumentList , ActualTypeArgument
     *</b>
     */
    struct ActualTypeArgumentList :public Ast
    {
        IAst* lpg_ActualTypeArgumentList;
        IAst* lpg_COMMA;
        IAst* lpg_ActualTypeArgument;

        IAst* getActualTypeArgumentList() { return lpg_ActualTypeArgumentList; };
        void setActualTypeArgumentList(IAst* lpg_ActualTypeArgumentList) { this->lpg_ActualTypeArgumentList = lpg_ActualTypeArgumentList; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getActualTypeArgument() { return lpg_ActualTypeArgument; };
        void setActualTypeArgument(IAst* lpg_ActualTypeArgument) { this->lpg_ActualTypeArgument = lpg_ActualTypeArgument; }

        ActualTypeArgumentList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ActualTypeArgumentList,
            IAst* lpg_COMMA,
            IAst* lpg_ActualTypeArgument) :Ast(leftIToken, rightIToken) {
            this->lpg_ActualTypeArgumentList = lpg_ActualTypeArgumentList;
            ((Ast*)lpg_ActualTypeArgumentList)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_ActualTypeArgument = lpg_ActualTypeArgument;
            ((Ast*)lpg_ActualTypeArgument)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ActualTypeArgumentList);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_ActualTypeArgument);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ActualTypeArgumentList->accept(v);
                lpg_COMMA->accept(v);
                lpg_ActualTypeArgument->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 44:  Wildcard ::= ? WildcardBoundsOpt
     *</b>
     */
    struct Wildcard :public Ast
    {
        IAst* lpg_QUESTION;
        IAst* lpg_WildcardBoundsOpt;

        IAst* getQUESTION() { return lpg_QUESTION; };
        void setQUESTION(IAst* lpg_QUESTION) { this->lpg_QUESTION = lpg_QUESTION; }
        /**
         * The value returned by <b>getWildcardBoundsOpt</b> may be <b>nullptr</b>
         */
        IAst* getWildcardBoundsOpt() { return lpg_WildcardBoundsOpt; };
        void setWildcardBoundsOpt(IAst* lpg_WildcardBoundsOpt) { this->lpg_WildcardBoundsOpt = lpg_WildcardBoundsOpt; }

        Wildcard(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_QUESTION,
            IAst* lpg_WildcardBoundsOpt) :Ast(leftIToken, rightIToken) {
            this->lpg_QUESTION = lpg_QUESTION;
            ((Ast*)lpg_QUESTION)->setParent(this);
            this->lpg_WildcardBoundsOpt = lpg_WildcardBoundsOpt;
            if (lpg_WildcardBoundsOpt != nullptr) ((Ast*)lpg_WildcardBoundsOpt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_QUESTION);
            list.push_back(lpg_WildcardBoundsOpt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_QUESTION->accept(v);
                if (lpg_WildcardBoundsOpt != nullptr) lpg_WildcardBoundsOpt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 47:  PackageName ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 48:  PackageName ::= PackageName . identifier
     *</b>
     */
    struct PackageName :public Ast
    {
        IAst* lpg_PackageName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getPackageName() { return lpg_PackageName; };
        void setPackageName(IAst* lpg_PackageName) { this->lpg_PackageName = lpg_PackageName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        PackageName(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PackageName,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_PackageName = lpg_PackageName;
            ((Ast*)lpg_PackageName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PackageName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PackageName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 49:  ExpressionName ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 50:  ExpressionName ::= AmbiguousName . identifier
     *</b>
     */
    struct ExpressionName :public Ast
    {
        IAst* lpg_AmbiguousName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getAmbiguousName() { return lpg_AmbiguousName; };
        void setAmbiguousName(IAst* lpg_AmbiguousName) { this->lpg_AmbiguousName = lpg_AmbiguousName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        ExpressionName(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AmbiguousName,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_AmbiguousName = lpg_AmbiguousName;
            ((Ast*)lpg_AmbiguousName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AmbiguousName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AmbiguousName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 51:  MethodName ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 52:  MethodName ::= AmbiguousName . identifier
     *</b>
     */
    struct MethodName :public Ast
    {
        IAst* lpg_AmbiguousName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getAmbiguousName() { return lpg_AmbiguousName; };
        void setAmbiguousName(IAst* lpg_AmbiguousName) { this->lpg_AmbiguousName = lpg_AmbiguousName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        MethodName(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AmbiguousName,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_AmbiguousName = lpg_AmbiguousName;
            ((Ast*)lpg_AmbiguousName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AmbiguousName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AmbiguousName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 53:  PackageOrTypeName ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 54:  PackageOrTypeName ::= PackageOrTypeName . identifier
     *</b>
     */
    struct PackageOrTypeName :public Ast
    {
        IAst* lpg_PackageOrTypeName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getPackageOrTypeName() { return lpg_PackageOrTypeName; };
        void setPackageOrTypeName(IAst* lpg_PackageOrTypeName) { this->lpg_PackageOrTypeName = lpg_PackageOrTypeName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        PackageOrTypeName(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PackageOrTypeName,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_PackageOrTypeName = lpg_PackageOrTypeName;
            ((Ast*)lpg_PackageOrTypeName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PackageOrTypeName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PackageOrTypeName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 55:  AmbiguousName ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 56:  AmbiguousName ::= AmbiguousName . identifier
     *</b>
     */
    struct AmbiguousName :public Ast
    {
        IAst* lpg_AmbiguousName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getAmbiguousName() { return lpg_AmbiguousName; };
        void setAmbiguousName(IAst* lpg_AmbiguousName) { this->lpg_AmbiguousName = lpg_AmbiguousName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        AmbiguousName(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AmbiguousName,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_AmbiguousName = lpg_AmbiguousName;
            ((Ast*)lpg_AmbiguousName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AmbiguousName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AmbiguousName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 57:  CompilationUnit ::= PackageDeclarationopt ImportDeclarationsopt TypeDeclarationsopt
     *</b>
     */
    struct CompilationUnit :public Ast
    {
        IAst* lpg_PackageDeclarationopt;
        IAst* lpg_ImportDeclarationsopt;
        IAst* lpg_TypeDeclarationsopt;

        /**
         * The value returned by <b>getPackageDeclarationopt</b> may be <b>nullptr</b>
         */
        IAst* getPackageDeclarationopt() { return lpg_PackageDeclarationopt; };
        void setPackageDeclarationopt(IAst* lpg_PackageDeclarationopt) { this->lpg_PackageDeclarationopt = lpg_PackageDeclarationopt; }
        /**
         * The value returned by <b>getImportDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getImportDeclarationsopt() { return lpg_ImportDeclarationsopt; };
        void setImportDeclarationsopt(IAst* lpg_ImportDeclarationsopt) { this->lpg_ImportDeclarationsopt = lpg_ImportDeclarationsopt; }
        /**
         * The value returned by <b>getTypeDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeDeclarationsopt() { return lpg_TypeDeclarationsopt; };
        void setTypeDeclarationsopt(IAst* lpg_TypeDeclarationsopt) { this->lpg_TypeDeclarationsopt = lpg_TypeDeclarationsopt; }

        CompilationUnit(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PackageDeclarationopt,
            IAst* lpg_ImportDeclarationsopt,
            IAst* lpg_TypeDeclarationsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_PackageDeclarationopt = lpg_PackageDeclarationopt;
            if (lpg_PackageDeclarationopt != nullptr) ((Ast*)lpg_PackageDeclarationopt)->setParent(this);
            this->lpg_ImportDeclarationsopt = lpg_ImportDeclarationsopt;
            if (lpg_ImportDeclarationsopt != nullptr) ((Ast*)lpg_ImportDeclarationsopt)->setParent(this);
            this->lpg_TypeDeclarationsopt = lpg_TypeDeclarationsopt;
            if (lpg_TypeDeclarationsopt != nullptr) ((Ast*)lpg_TypeDeclarationsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PackageDeclarationopt);
            list.push_back(lpg_ImportDeclarationsopt);
            list.push_back(lpg_TypeDeclarationsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_PackageDeclarationopt != nullptr) lpg_PackageDeclarationopt->accept(v);
                if (lpg_ImportDeclarationsopt != nullptr) lpg_ImportDeclarationsopt->accept(v);
                if (lpg_TypeDeclarationsopt != nullptr) lpg_TypeDeclarationsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 58:  ImportDeclarations ::= ImportDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 59:  ImportDeclarations ::= ImportDeclarations ImportDeclaration
     *</b>
     */
    struct ImportDeclarations :public Ast
    {
        IAst* lpg_ImportDeclarations;
        IAst* lpg_ImportDeclaration;

        IAst* getImportDeclarations() { return lpg_ImportDeclarations; };
        void setImportDeclarations(IAst* lpg_ImportDeclarations) { this->lpg_ImportDeclarations = lpg_ImportDeclarations; }
        IAst* getImportDeclaration() { return lpg_ImportDeclaration; };
        void setImportDeclaration(IAst* lpg_ImportDeclaration) { this->lpg_ImportDeclaration = lpg_ImportDeclaration; }

        ImportDeclarations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ImportDeclarations,
            IAst* lpg_ImportDeclaration) :Ast(leftIToken, rightIToken) {
            this->lpg_ImportDeclarations = lpg_ImportDeclarations;
            ((Ast*)lpg_ImportDeclarations)->setParent(this);
            this->lpg_ImportDeclaration = lpg_ImportDeclaration;
            ((Ast*)lpg_ImportDeclaration)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ImportDeclarations);
            list.push_back(lpg_ImportDeclaration);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ImportDeclarations->accept(v);
                lpg_ImportDeclaration->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 60:  TypeDeclarations ::= TypeDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 61:  TypeDeclarations ::= TypeDeclarations TypeDeclaration
     *</b>
     */
    struct TypeDeclarations :public Ast
    {
        IAst* lpg_TypeDeclarations;
        IAst* lpg_TypeDeclaration;

        IAst* getTypeDeclarations() { return lpg_TypeDeclarations; };
        void setTypeDeclarations(IAst* lpg_TypeDeclarations) { this->lpg_TypeDeclarations = lpg_TypeDeclarations; }
        IAst* getTypeDeclaration() { return lpg_TypeDeclaration; };
        void setTypeDeclaration(IAst* lpg_TypeDeclaration) { this->lpg_TypeDeclaration = lpg_TypeDeclaration; }

        TypeDeclarations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeDeclarations,
            IAst* lpg_TypeDeclaration) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeDeclarations = lpg_TypeDeclarations;
            ((Ast*)lpg_TypeDeclarations)->setParent(this);
            this->lpg_TypeDeclaration = lpg_TypeDeclaration;
            ((Ast*)lpg_TypeDeclaration)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeDeclarations);
            list.push_back(lpg_TypeDeclaration);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeDeclarations->accept(v);
                lpg_TypeDeclaration->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 62:  PackageDeclaration ::= Annotationsopt package PackageName ;
     *</b>
     */
    struct PackageDeclaration :public Ast
    {
        IAst* lpg_Annotationsopt;
        IAst* lpg_package;
        IAst* lpg_PackageName;
        IAst* lpg_SEMICOLON;

        /**
         * The value returned by <b>getAnnotationsopt</b> may be <b>nullptr</b>
         */
        IAst* getAnnotationsopt() { return lpg_Annotationsopt; };
        void setAnnotationsopt(IAst* lpg_Annotationsopt) { this->lpg_Annotationsopt = lpg_Annotationsopt; }
        IAst* getpackage() { return lpg_package; };
        void setpackage(IAst* lpg_package) { this->lpg_package = lpg_package; }
        IAst* getPackageName() { return lpg_PackageName; };
        void setPackageName(IAst* lpg_PackageName) { this->lpg_PackageName = lpg_PackageName; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        PackageDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Annotationsopt,
            IAst* lpg_package,
            IAst* lpg_PackageName,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_Annotationsopt = lpg_Annotationsopt;
            if (lpg_Annotationsopt != nullptr) ((Ast*)lpg_Annotationsopt)->setParent(this);
            this->lpg_package = lpg_package;
            ((Ast*)lpg_package)->setParent(this);
            this->lpg_PackageName = lpg_PackageName;
            ((Ast*)lpg_PackageName)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Annotationsopt);
            list.push_back(lpg_package);
            list.push_back(lpg_PackageName);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_Annotationsopt != nullptr) lpg_Annotationsopt->accept(v);
                lpg_package->accept(v);
                lpg_PackageName->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 67:  SingleTypeImportDeclaration ::= import TypeName ;
     *</b>
     */
    struct SingleTypeImportDeclaration :public Ast
    {
        IAst* lpg_import;
        IAst* lpg_TypeName;
        IAst* lpg_SEMICOLON;

        IAst* getimport() { return lpg_import; };
        void setimport(IAst* lpg_import) { this->lpg_import = lpg_import; }
        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        SingleTypeImportDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_import,
            IAst* lpg_TypeName,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_import = lpg_import;
            ((Ast*)lpg_import)->setParent(this);
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_import);
            list.push_back(lpg_TypeName);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_import->accept(v);
                lpg_TypeName->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 68:  TypeImportOnDemandDeclaration ::= import PackageOrTypeName . * ;
     *</b>
     */
    struct TypeImportOnDemandDeclaration :public Ast
    {
        IAst* lpg_import;
        IAst* lpg_PackageOrTypeName;
        IAst* lpg_DOT;
        IAst* lpg_MULTIPLY;
        IAst* lpg_SEMICOLON;

        IAst* getimport() { return lpg_import; };
        void setimport(IAst* lpg_import) { this->lpg_import = lpg_import; }
        IAst* getPackageOrTypeName() { return lpg_PackageOrTypeName; };
        void setPackageOrTypeName(IAst* lpg_PackageOrTypeName) { this->lpg_PackageOrTypeName = lpg_PackageOrTypeName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getMULTIPLY() { return lpg_MULTIPLY; };
        void setMULTIPLY(IAst* lpg_MULTIPLY) { this->lpg_MULTIPLY = lpg_MULTIPLY; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        TypeImportOnDemandDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_import,
            IAst* lpg_PackageOrTypeName,
            IAst* lpg_DOT,
            IAst* lpg_MULTIPLY,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_import = lpg_import;
            ((Ast*)lpg_import)->setParent(this);
            this->lpg_PackageOrTypeName = lpg_PackageOrTypeName;
            ((Ast*)lpg_PackageOrTypeName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_MULTIPLY = lpg_MULTIPLY;
            ((Ast*)lpg_MULTIPLY)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_import);
            list.push_back(lpg_PackageOrTypeName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_MULTIPLY);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_import->accept(v);
                lpg_PackageOrTypeName->accept(v);
                lpg_DOT->accept(v);
                lpg_MULTIPLY->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 69:  SingleStaticImportDeclaration ::= import static TypeName . identifier ;
     *</b>
     */
    struct SingleStaticImportDeclaration :public Ast
    {
        IAst* lpg_import;
        IAst* lpg_static;
        IAst* lpg_TypeName;
        IAst* lpg_DOT;
        IAst* lpg_identifier;
        IAst* lpg_SEMICOLON;

        IAst* getimport() { return lpg_import; };
        void setimport(IAst* lpg_import) { this->lpg_import = lpg_import; }
        IAst* getstatic() { return lpg_static; };
        void setstatic(IAst* lpg_static) { this->lpg_static = lpg_static; }
        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        SingleStaticImportDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_import,
            IAst* lpg_static,
            IAst* lpg_TypeName,
            IAst* lpg_DOT,
            IAst* lpg_identifier,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_import = lpg_import;
            ((Ast*)lpg_import)->setParent(this);
            this->lpg_static = lpg_static;
            ((Ast*)lpg_static)->setParent(this);
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_import);
            list.push_back(lpg_static);
            list.push_back(lpg_TypeName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_import->accept(v);
                lpg_static->accept(v);
                lpg_TypeName->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 70:  StaticImportOnDemandDeclaration ::= import static TypeName . * ;
     *</b>
     */
    struct StaticImportOnDemandDeclaration :public Ast
    {
        IAst* lpg_import;
        IAst* lpg_static;
        IAst* lpg_TypeName;
        IAst* lpg_DOT;
        IAst* lpg_MULTIPLY;
        IAst* lpg_SEMICOLON;

        IAst* getimport() { return lpg_import; };
        void setimport(IAst* lpg_import) { this->lpg_import = lpg_import; }
        IAst* getstatic() { return lpg_static; };
        void setstatic(IAst* lpg_static) { this->lpg_static = lpg_static; }
        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getMULTIPLY() { return lpg_MULTIPLY; };
        void setMULTIPLY(IAst* lpg_MULTIPLY) { this->lpg_MULTIPLY = lpg_MULTIPLY; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        StaticImportOnDemandDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_import,
            IAst* lpg_static,
            IAst* lpg_TypeName,
            IAst* lpg_DOT,
            IAst* lpg_MULTIPLY,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_import = lpg_import;
            ((Ast*)lpg_import)->setParent(this);
            this->lpg_static = lpg_static;
            ((Ast*)lpg_static)->setParent(this);
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_MULTIPLY = lpg_MULTIPLY;
            ((Ast*)lpg_MULTIPLY)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_import);
            list.push_back(lpg_static);
            list.push_back(lpg_TypeName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_MULTIPLY);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_import->accept(v);
                lpg_static->accept(v);
                lpg_TypeName->accept(v);
                lpg_DOT->accept(v);
                lpg_MULTIPLY->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 71:  TypeDeclaration ::= ClassDeclaration
     *<li>Rule 72:  TypeDeclaration ::= InterfaceDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 73:  TypeDeclaration ::= ;
     *</b>
     */
    struct TypeDeclaration :public AstToken
    {
        IToken* getSEMICOLON() { return leftIToken; }

        TypeDeclaration(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 76:  NormalClassDeclaration ::= ClassModifiersopt class identifier TypeParametersopt Superopt Interfacesopt ClassBody
     *</b>
     */
    struct NormalClassDeclaration :public Ast
    {
        IAst* lpg_ClassModifiersopt;
        IAst* lpg_class;
        IAst* lpg_identifier;
        IAst* lpg_TypeParametersopt;
        IAst* lpg_Superopt;
        IAst* lpg_Interfacesopt;
        IAst* lpg_ClassBody;

        /**
         * The value returned by <b>getClassModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getClassModifiersopt() { return lpg_ClassModifiersopt; };
        void setClassModifiersopt(IAst* lpg_ClassModifiersopt) { this->lpg_ClassModifiersopt = lpg_ClassModifiersopt; }
        IAst* getclass() { return lpg_class; };
        void setclass(IAst* lpg_class) { this->lpg_class = lpg_class; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        /**
         * The value returned by <b>getTypeParametersopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeParametersopt() { return lpg_TypeParametersopt; };
        void setTypeParametersopt(IAst* lpg_TypeParametersopt) { this->lpg_TypeParametersopt = lpg_TypeParametersopt; }
        /**
         * The value returned by <b>getSuperopt</b> may be <b>nullptr</b>
         */
        IAst* getSuperopt() { return lpg_Superopt; };
        void setSuperopt(IAst* lpg_Superopt) { this->lpg_Superopt = lpg_Superopt; }
        /**
         * The value returned by <b>getInterfacesopt</b> may be <b>nullptr</b>
         */
        IAst* getInterfacesopt() { return lpg_Interfacesopt; };
        void setInterfacesopt(IAst* lpg_Interfacesopt) { this->lpg_Interfacesopt = lpg_Interfacesopt; }
        IAst* getClassBody() { return lpg_ClassBody; };
        void setClassBody(IAst* lpg_ClassBody) { this->lpg_ClassBody = lpg_ClassBody; }

        NormalClassDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassModifiersopt,
            IAst* lpg_class,
            IAst* lpg_identifier,
            IAst* lpg_TypeParametersopt,
            IAst* lpg_Superopt,
            IAst* lpg_Interfacesopt,
            IAst* lpg_ClassBody) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassModifiersopt = lpg_ClassModifiersopt;
            if (lpg_ClassModifiersopt != nullptr) ((Ast*)lpg_ClassModifiersopt)->setParent(this);
            this->lpg_class = lpg_class;
            ((Ast*)lpg_class)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_TypeParametersopt = lpg_TypeParametersopt;
            if (lpg_TypeParametersopt != nullptr) ((Ast*)lpg_TypeParametersopt)->setParent(this);
            this->lpg_Superopt = lpg_Superopt;
            if (lpg_Superopt != nullptr) ((Ast*)lpg_Superopt)->setParent(this);
            this->lpg_Interfacesopt = lpg_Interfacesopt;
            if (lpg_Interfacesopt != nullptr) ((Ast*)lpg_Interfacesopt)->setParent(this);
            this->lpg_ClassBody = lpg_ClassBody;
            ((Ast*)lpg_ClassBody)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassModifiersopt);
            list.push_back(lpg_class);
            list.push_back(lpg_identifier);
            list.push_back(lpg_TypeParametersopt);
            list.push_back(lpg_Superopt);
            list.push_back(lpg_Interfacesopt);
            list.push_back(lpg_ClassBody);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_ClassModifiersopt != nullptr) lpg_ClassModifiersopt->accept(v);
                lpg_class->accept(v);
                lpg_identifier->accept(v);
                if (lpg_TypeParametersopt != nullptr) lpg_TypeParametersopt->accept(v);
                if (lpg_Superopt != nullptr) lpg_Superopt->accept(v);
                if (lpg_Interfacesopt != nullptr) lpg_Interfacesopt->accept(v);
                lpg_ClassBody->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 77:  ClassModifiers ::= ClassModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 78:  ClassModifiers ::= ClassModifiers ClassModifier
     *</b>
     */
    struct ClassModifiers :public Ast
    {
        IAst* lpg_ClassModifiers;
        IAst* lpg_ClassModifier;

        IAst* getClassModifiers() { return lpg_ClassModifiers; };
        void setClassModifiers(IAst* lpg_ClassModifiers) { this->lpg_ClassModifiers = lpg_ClassModifiers; }
        IAst* getClassModifier() { return lpg_ClassModifier; };
        void setClassModifier(IAst* lpg_ClassModifier) { this->lpg_ClassModifier = lpg_ClassModifier; }

        ClassModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassModifiers,
            IAst* lpg_ClassModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassModifiers = lpg_ClassModifiers;
            ((Ast*)lpg_ClassModifiers)->setParent(this);
            this->lpg_ClassModifier = lpg_ClassModifier;
            ((Ast*)lpg_ClassModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassModifiers);
            list.push_back(lpg_ClassModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ClassModifiers->accept(v);
                lpg_ClassModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 87:  TypeParameters ::= < TypeParameterList >
     *</b>
     */
    struct TypeParameters :public Ast
    {
        IAst* lpg_LESS;
        IAst* lpg_TypeParameterList;
        IAst* lpg_GREATER;

        IAst* getLESS() { return lpg_LESS; };
        void setLESS(IAst* lpg_LESS) { this->lpg_LESS = lpg_LESS; }
        IAst* getTypeParameterList() { return lpg_TypeParameterList; };
        void setTypeParameterList(IAst* lpg_TypeParameterList) { this->lpg_TypeParameterList = lpg_TypeParameterList; }
        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }

        TypeParameters(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LESS,
            IAst* lpg_TypeParameterList,
            IAst* lpg_GREATER) :Ast(leftIToken, rightIToken) {
            this->lpg_LESS = lpg_LESS;
            ((Ast*)lpg_LESS)->setParent(this);
            this->lpg_TypeParameterList = lpg_TypeParameterList;
            ((Ast*)lpg_TypeParameterList)->setParent(this);
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LESS);
            list.push_back(lpg_TypeParameterList);
            list.push_back(lpg_GREATER);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LESS->accept(v);
                lpg_TypeParameterList->accept(v);
                lpg_GREATER->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 88:  TypeParameterList ::= TypeParameter
     *</em>
     *<p>
     *<b>
     *<li>Rule 89:  TypeParameterList ::= TypeParameterList , TypeParameter
     *</b>
     */
    struct TypeParameterList :public Ast
    {
        IAst* lpg_TypeParameterList;
        IAst* lpg_COMMA;
        IAst* lpg_TypeParameter;

        IAst* getTypeParameterList() { return lpg_TypeParameterList; };
        void setTypeParameterList(IAst* lpg_TypeParameterList) { this->lpg_TypeParameterList = lpg_TypeParameterList; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getTypeParameter() { return lpg_TypeParameter; };
        void setTypeParameter(IAst* lpg_TypeParameter) { this->lpg_TypeParameter = lpg_TypeParameter; }

        TypeParameterList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeParameterList,
            IAst* lpg_COMMA,
            IAst* lpg_TypeParameter) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeParameterList = lpg_TypeParameterList;
            ((Ast*)lpg_TypeParameterList)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_TypeParameter = lpg_TypeParameter;
            ((Ast*)lpg_TypeParameter)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeParameterList);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_TypeParameter);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeParameterList->accept(v);
                lpg_COMMA->accept(v);
                lpg_TypeParameter->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 90:  Super ::= extends ClassType
     *</b>
     */
    struct Super :public Ast
    {
        IAst* lpg_extends;
        IAst* lpg_ClassType;

        IAst* getextends() { return lpg_extends; };
        void setextends(IAst* lpg_extends) { this->lpg_extends = lpg_extends; }
        IAst* getClassType() { return lpg_ClassType; };
        void setClassType(IAst* lpg_ClassType) { this->lpg_ClassType = lpg_ClassType; }

        Super(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_extends,
            IAst* lpg_ClassType) :Ast(leftIToken, rightIToken) {
            this->lpg_extends = lpg_extends;
            ((Ast*)lpg_extends)->setParent(this);
            this->lpg_ClassType = lpg_ClassType;
            ((Ast*)lpg_ClassType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_extends);
            list.push_back(lpg_ClassType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_extends->accept(v);
                lpg_ClassType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 91:  Interfaces ::= implements InterfaceTypeList
     *</b>
     */
    struct Interfaces :public Ast
    {
        IAst* lpg_implements;
        IAst* lpg_InterfaceTypeList;

        IAst* getimplements() { return lpg_implements; };
        void setimplements(IAst* lpg_implements) { this->lpg_implements = lpg_implements; }
        IAst* getInterfaceTypeList() { return lpg_InterfaceTypeList; };
        void setInterfaceTypeList(IAst* lpg_InterfaceTypeList) { this->lpg_InterfaceTypeList = lpg_InterfaceTypeList; }

        Interfaces(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_implements,
            IAst* lpg_InterfaceTypeList) :Ast(leftIToken, rightIToken) {
            this->lpg_implements = lpg_implements;
            ((Ast*)lpg_implements)->setParent(this);
            this->lpg_InterfaceTypeList = lpg_InterfaceTypeList;
            ((Ast*)lpg_InterfaceTypeList)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_implements);
            list.push_back(lpg_InterfaceTypeList);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_implements->accept(v);
                lpg_InterfaceTypeList->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 92:  InterfaceTypeList ::= InterfaceType
     *</em>
     *<p>
     *<b>
     *<li>Rule 93:  InterfaceTypeList ::= InterfaceTypeList , InterfaceType
     *</b>
     */
    struct InterfaceTypeList :public Ast
    {
        IAst* lpg_InterfaceTypeList;
        IAst* lpg_COMMA;
        IAst* lpg_InterfaceType;

        IAst* getInterfaceTypeList() { return lpg_InterfaceTypeList; };
        void setInterfaceTypeList(IAst* lpg_InterfaceTypeList) { this->lpg_InterfaceTypeList = lpg_InterfaceTypeList; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getInterfaceType() { return lpg_InterfaceType; };
        void setInterfaceType(IAst* lpg_InterfaceType) { this->lpg_InterfaceType = lpg_InterfaceType; }

        InterfaceTypeList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_InterfaceTypeList,
            IAst* lpg_COMMA,
            IAst* lpg_InterfaceType) :Ast(leftIToken, rightIToken) {
            this->lpg_InterfaceTypeList = lpg_InterfaceTypeList;
            ((Ast*)lpg_InterfaceTypeList)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_InterfaceType = lpg_InterfaceType;
            ((Ast*)lpg_InterfaceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_InterfaceTypeList);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_InterfaceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_InterfaceTypeList->accept(v);
                lpg_COMMA->accept(v);
                lpg_InterfaceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 94:  ClassBody ::= { ClassBodyDeclarationsopt }
     *</b>
     */
    struct ClassBody :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_ClassBodyDeclarationsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getClassBodyDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getClassBodyDeclarationsopt() { return lpg_ClassBodyDeclarationsopt; };
        void setClassBodyDeclarationsopt(IAst* lpg_ClassBodyDeclarationsopt) { this->lpg_ClassBodyDeclarationsopt = lpg_ClassBodyDeclarationsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        ClassBody(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_ClassBodyDeclarationsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_ClassBodyDeclarationsopt = lpg_ClassBodyDeclarationsopt;
            if (lpg_ClassBodyDeclarationsopt != nullptr) ((Ast*)lpg_ClassBodyDeclarationsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_ClassBodyDeclarationsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_ClassBodyDeclarationsopt != nullptr) lpg_ClassBodyDeclarationsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 95:  ClassBodyDeclarations ::= ClassBodyDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 96:  ClassBodyDeclarations ::= ClassBodyDeclarations ClassBodyDeclaration
     *</b>
     */
    struct ClassBodyDeclarations :public Ast
    {
        IAst* lpg_ClassBodyDeclarations;
        IAst* lpg_ClassBodyDeclaration;

        IAst* getClassBodyDeclarations() { return lpg_ClassBodyDeclarations; };
        void setClassBodyDeclarations(IAst* lpg_ClassBodyDeclarations) { this->lpg_ClassBodyDeclarations = lpg_ClassBodyDeclarations; }
        IAst* getClassBodyDeclaration() { return lpg_ClassBodyDeclaration; };
        void setClassBodyDeclaration(IAst* lpg_ClassBodyDeclaration) { this->lpg_ClassBodyDeclaration = lpg_ClassBodyDeclaration; }

        ClassBodyDeclarations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassBodyDeclarations,
            IAst* lpg_ClassBodyDeclaration) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassBodyDeclarations = lpg_ClassBodyDeclarations;
            ((Ast*)lpg_ClassBodyDeclarations)->setParent(this);
            this->lpg_ClassBodyDeclaration = lpg_ClassBodyDeclaration;
            ((Ast*)lpg_ClassBodyDeclaration)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassBodyDeclarations);
            list.push_back(lpg_ClassBodyDeclaration);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ClassBodyDeclarations->accept(v);
                lpg_ClassBodyDeclaration->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 101:  ClassMemberDeclaration ::= FieldDeclaration
     *<li>Rule 102:  ClassMemberDeclaration ::= MethodDeclaration
     *<li>Rule 103:  ClassMemberDeclaration ::= ClassDeclaration
     *<li>Rule 104:  ClassMemberDeclaration ::= InterfaceDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 105:  ClassMemberDeclaration ::= ;
     *</b>
     */
    struct ClassMemberDeclaration :public AstToken
    {
        IToken* getSEMICOLON() { return leftIToken; }

        ClassMemberDeclaration(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 106:  FieldDeclaration ::= FieldModifiersopt Type VariableDeclarators ;
     *</b>
     */
    struct FieldDeclaration :public Ast
    {
        IAst* lpg_FieldModifiersopt;
        IAst* lpg_Type;
        IAst* lpg_VariableDeclarators;
        IAst* lpg_SEMICOLON;

        /**
         * The value returned by <b>getFieldModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getFieldModifiersopt() { return lpg_FieldModifiersopt; };
        void setFieldModifiersopt(IAst* lpg_FieldModifiersopt) { this->lpg_FieldModifiersopt = lpg_FieldModifiersopt; }
        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getVariableDeclarators() { return lpg_VariableDeclarators; };
        void setVariableDeclarators(IAst* lpg_VariableDeclarators) { this->lpg_VariableDeclarators = lpg_VariableDeclarators; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        FieldDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_FieldModifiersopt,
            IAst* lpg_Type,
            IAst* lpg_VariableDeclarators,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_FieldModifiersopt = lpg_FieldModifiersopt;
            if (lpg_FieldModifiersopt != nullptr) ((Ast*)lpg_FieldModifiersopt)->setParent(this);
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_VariableDeclarators = lpg_VariableDeclarators;
            ((Ast*)lpg_VariableDeclarators)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_FieldModifiersopt);
            list.push_back(lpg_Type);
            list.push_back(lpg_VariableDeclarators);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_FieldModifiersopt != nullptr) lpg_FieldModifiersopt->accept(v);
                lpg_Type->accept(v);
                lpg_VariableDeclarators->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 107:  VariableDeclarators ::= VariableDeclarator
     *</em>
     *<p>
     *<b>
     *<li>Rule 108:  VariableDeclarators ::= VariableDeclarators , VariableDeclarator
     *</b>
     */
    struct VariableDeclarators :public Ast
    {
        IAst* lpg_VariableDeclarators;
        IAst* lpg_COMMA;
        IAst* lpg_VariableDeclarator;

        IAst* getVariableDeclarators() { return lpg_VariableDeclarators; };
        void setVariableDeclarators(IAst* lpg_VariableDeclarators) { this->lpg_VariableDeclarators = lpg_VariableDeclarators; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getVariableDeclarator() { return lpg_VariableDeclarator; };
        void setVariableDeclarator(IAst* lpg_VariableDeclarator) { this->lpg_VariableDeclarator = lpg_VariableDeclarator; }

        VariableDeclarators(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableDeclarators,
            IAst* lpg_COMMA,
            IAst* lpg_VariableDeclarator) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableDeclarators = lpg_VariableDeclarators;
            ((Ast*)lpg_VariableDeclarators)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_VariableDeclarator = lpg_VariableDeclarator;
            ((Ast*)lpg_VariableDeclarator)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableDeclarators);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_VariableDeclarator);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_VariableDeclarators->accept(v);
                lpg_COMMA->accept(v);
                lpg_VariableDeclarator->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 109:  VariableDeclarator ::= VariableDeclaratorId
     *</em>
     *<p>
     *<b>
     *<li>Rule 110:  VariableDeclarator ::= VariableDeclaratorId = VariableInitializer
     *</b>
     */
    struct VariableDeclarator :public Ast
    {
        IAst* lpg_VariableDeclaratorId;
        IAst* lpg_EQUAL;
        IAst* lpg_VariableInitializer;

        IAst* getVariableDeclaratorId() { return lpg_VariableDeclaratorId; };
        void setVariableDeclaratorId(IAst* lpg_VariableDeclaratorId) { this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId; }
        IAst* getEQUAL() { return lpg_EQUAL; };
        void setEQUAL(IAst* lpg_EQUAL) { this->lpg_EQUAL = lpg_EQUAL; }
        IAst* getVariableInitializer() { return lpg_VariableInitializer; };
        void setVariableInitializer(IAst* lpg_VariableInitializer) { this->lpg_VariableInitializer = lpg_VariableInitializer; }

        VariableDeclarator(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableDeclaratorId,
            IAst* lpg_EQUAL,
            IAst* lpg_VariableInitializer) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId;
            ((Ast*)lpg_VariableDeclaratorId)->setParent(this);
            this->lpg_EQUAL = lpg_EQUAL;
            ((Ast*)lpg_EQUAL)->setParent(this);
            this->lpg_VariableInitializer = lpg_VariableInitializer;
            ((Ast*)lpg_VariableInitializer)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableDeclaratorId);
            list.push_back(lpg_EQUAL);
            list.push_back(lpg_VariableInitializer);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_VariableDeclaratorId->accept(v);
                lpg_EQUAL->accept(v);
                lpg_VariableInitializer->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 111:  VariableDeclaratorId ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 112:  VariableDeclaratorId ::= VariableDeclaratorId [ ]
     *</b>
     */
    struct VariableDeclaratorId :public Ast
    {
        IAst* lpg_VariableDeclaratorId;
        IAst* lpg_LBRACKET;
        IAst* lpg_RBRACKET;

        IAst* getVariableDeclaratorId() { return lpg_VariableDeclaratorId; };
        void setVariableDeclaratorId(IAst* lpg_VariableDeclaratorId) { this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId; }
        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        VariableDeclaratorId(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableDeclaratorId,
            IAst* lpg_LBRACKET,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId;
            ((Ast*)lpg_VariableDeclaratorId)->setParent(this);
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableDeclaratorId);
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_VariableDeclaratorId->accept(v);
                lpg_LBRACKET->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 115:  FieldModifiers ::= FieldModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 116:  FieldModifiers ::= FieldModifiers FieldModifier
     *</b>
     */
    struct FieldModifiers :public Ast
    {
        IAst* lpg_FieldModifiers;
        IAst* lpg_FieldModifier;

        IAst* getFieldModifiers() { return lpg_FieldModifiers; };
        void setFieldModifiers(IAst* lpg_FieldModifiers) { this->lpg_FieldModifiers = lpg_FieldModifiers; }
        IAst* getFieldModifier() { return lpg_FieldModifier; };
        void setFieldModifier(IAst* lpg_FieldModifier) { this->lpg_FieldModifier = lpg_FieldModifier; }

        FieldModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_FieldModifiers,
            IAst* lpg_FieldModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_FieldModifiers = lpg_FieldModifiers;
            ((Ast*)lpg_FieldModifiers)->setParent(this);
            this->lpg_FieldModifier = lpg_FieldModifier;
            ((Ast*)lpg_FieldModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_FieldModifiers);
            list.push_back(lpg_FieldModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_FieldModifiers->accept(v);
                lpg_FieldModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 125:  MethodDeclaration ::= MethodHeader MethodBody
     *</b>
     */
    struct MethodDeclaration :public Ast
    {
        IAst* lpg_MethodHeader;
        IAst* lpg_MethodBody;

        IAst* getMethodHeader() { return lpg_MethodHeader; };
        void setMethodHeader(IAst* lpg_MethodHeader) { this->lpg_MethodHeader = lpg_MethodHeader; }
        IAst* getMethodBody() { return lpg_MethodBody; };
        void setMethodBody(IAst* lpg_MethodBody) { this->lpg_MethodBody = lpg_MethodBody; }

        MethodDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MethodHeader,
            IAst* lpg_MethodBody) :Ast(leftIToken, rightIToken) {
            this->lpg_MethodHeader = lpg_MethodHeader;
            ((Ast*)lpg_MethodHeader)->setParent(this);
            this->lpg_MethodBody = lpg_MethodBody;
            ((Ast*)lpg_MethodBody)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MethodHeader);
            list.push_back(lpg_MethodBody);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MethodHeader->accept(v);
                lpg_MethodBody->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 126:  MethodHeader ::= MethodModifiersopt TypeParametersopt ResultType MethodDeclarator Throwsopt
     *</b>
     */
    struct MethodHeader :public Ast
    {
        IAst* lpg_MethodModifiersopt;
        IAst* lpg_TypeParametersopt;
        IAst* lpg_ResultType;
        IAst* lpg_MethodDeclarator;
        IAst* lpg_Throwsopt;

        /**
         * The value returned by <b>getMethodModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getMethodModifiersopt() { return lpg_MethodModifiersopt; };
        void setMethodModifiersopt(IAst* lpg_MethodModifiersopt) { this->lpg_MethodModifiersopt = lpg_MethodModifiersopt; }
        /**
         * The value returned by <b>getTypeParametersopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeParametersopt() { return lpg_TypeParametersopt; };
        void setTypeParametersopt(IAst* lpg_TypeParametersopt) { this->lpg_TypeParametersopt = lpg_TypeParametersopt; }
        IAst* getResultType() { return lpg_ResultType; };
        void setResultType(IAst* lpg_ResultType) { this->lpg_ResultType = lpg_ResultType; }
        IAst* getMethodDeclarator() { return lpg_MethodDeclarator; };
        void setMethodDeclarator(IAst* lpg_MethodDeclarator) { this->lpg_MethodDeclarator = lpg_MethodDeclarator; }
        /**
         * The value returned by <b>getThrowsopt</b> may be <b>nullptr</b>
         */
        IAst* getThrowsopt() { return lpg_Throwsopt; };
        void setThrowsopt(IAst* lpg_Throwsopt) { this->lpg_Throwsopt = lpg_Throwsopt; }

        MethodHeader(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MethodModifiersopt,
            IAst* lpg_TypeParametersopt,
            IAst* lpg_ResultType,
            IAst* lpg_MethodDeclarator,
            IAst* lpg_Throwsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_MethodModifiersopt = lpg_MethodModifiersopt;
            if (lpg_MethodModifiersopt != nullptr) ((Ast*)lpg_MethodModifiersopt)->setParent(this);
            this->lpg_TypeParametersopt = lpg_TypeParametersopt;
            if (lpg_TypeParametersopt != nullptr) ((Ast*)lpg_TypeParametersopt)->setParent(this);
            this->lpg_ResultType = lpg_ResultType;
            ((Ast*)lpg_ResultType)->setParent(this);
            this->lpg_MethodDeclarator = lpg_MethodDeclarator;
            ((Ast*)lpg_MethodDeclarator)->setParent(this);
            this->lpg_Throwsopt = lpg_Throwsopt;
            if (lpg_Throwsopt != nullptr) ((Ast*)lpg_Throwsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MethodModifiersopt);
            list.push_back(lpg_TypeParametersopt);
            list.push_back(lpg_ResultType);
            list.push_back(lpg_MethodDeclarator);
            list.push_back(lpg_Throwsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_MethodModifiersopt != nullptr) lpg_MethodModifiersopt->accept(v);
                if (lpg_TypeParametersopt != nullptr) lpg_TypeParametersopt->accept(v);
                lpg_ResultType->accept(v);
                lpg_MethodDeclarator->accept(v);
                if (lpg_Throwsopt != nullptr) lpg_Throwsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 127:  ResultType ::= Type
     *</em>
     *<p>
     *<b>
     *<li>Rule 128:  ResultType ::= void
     *</b>
     */
    struct ResultType :public AstToken
    {
        IToken* getvoid() { return leftIToken; }

        ResultType(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 131:  FormalParameterList ::= LastFormalParameter
     *</em>
     *<p>
     *<b>
     *<li>Rule 132:  FormalParameterList ::= FormalParameters , LastFormalParameter
     *</b>
     */
    struct FormalParameterList :public Ast
    {
        IAst* lpg_FormalParameters;
        IAst* lpg_COMMA;
        IAst* lpg_LastFormalParameter;

        IAst* getFormalParameters() { return lpg_FormalParameters; };
        void setFormalParameters(IAst* lpg_FormalParameters) { this->lpg_FormalParameters = lpg_FormalParameters; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getLastFormalParameter() { return lpg_LastFormalParameter; };
        void setLastFormalParameter(IAst* lpg_LastFormalParameter) { this->lpg_LastFormalParameter = lpg_LastFormalParameter; }

        FormalParameterList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_FormalParameters,
            IAst* lpg_COMMA,
            IAst* lpg_LastFormalParameter) :Ast(leftIToken, rightIToken) {
            this->lpg_FormalParameters = lpg_FormalParameters;
            ((Ast*)lpg_FormalParameters)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_LastFormalParameter = lpg_LastFormalParameter;
            ((Ast*)lpg_LastFormalParameter)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_FormalParameters);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_LastFormalParameter);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_FormalParameters->accept(v);
                lpg_COMMA->accept(v);
                lpg_LastFormalParameter->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 133:  FormalParameters ::= FormalParameter
     *</em>
     *<p>
     *<b>
     *<li>Rule 134:  FormalParameters ::= FormalParameters , FormalParameter
     *</b>
     */
    struct FormalParameters :public Ast
    {
        IAst* lpg_FormalParameters;
        IAst* lpg_COMMA;
        IAst* lpg_FormalParameter;

        IAst* getFormalParameters() { return lpg_FormalParameters; };
        void setFormalParameters(IAst* lpg_FormalParameters) { this->lpg_FormalParameters = lpg_FormalParameters; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getFormalParameter() { return lpg_FormalParameter; };
        void setFormalParameter(IAst* lpg_FormalParameter) { this->lpg_FormalParameter = lpg_FormalParameter; }

        FormalParameters(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_FormalParameters,
            IAst* lpg_COMMA,
            IAst* lpg_FormalParameter) :Ast(leftIToken, rightIToken) {
            this->lpg_FormalParameters = lpg_FormalParameters;
            ((Ast*)lpg_FormalParameters)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_FormalParameter = lpg_FormalParameter;
            ((Ast*)lpg_FormalParameter)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_FormalParameters);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_FormalParameter);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_FormalParameters->accept(v);
                lpg_COMMA->accept(v);
                lpg_FormalParameter->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 135:  FormalParameter ::= VariableModifiersopt Type VariableDeclaratorId
     *</b>
     */
    struct FormalParameter :public Ast
    {
        IAst* lpg_VariableModifiersopt;
        IAst* lpg_Type;
        IAst* lpg_VariableDeclaratorId;

        /**
         * The value returned by <b>getVariableModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getVariableModifiersopt() { return lpg_VariableModifiersopt; };
        void setVariableModifiersopt(IAst* lpg_VariableModifiersopt) { this->lpg_VariableModifiersopt = lpg_VariableModifiersopt; }
        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getVariableDeclaratorId() { return lpg_VariableDeclaratorId; };
        void setVariableDeclaratorId(IAst* lpg_VariableDeclaratorId) { this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId; }

        FormalParameter(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableModifiersopt,
            IAst* lpg_Type,
            IAst* lpg_VariableDeclaratorId) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableModifiersopt = lpg_VariableModifiersopt;
            if (lpg_VariableModifiersopt != nullptr) ((Ast*)lpg_VariableModifiersopt)->setParent(this);
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId;
            ((Ast*)lpg_VariableDeclaratorId)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableModifiersopt);
            list.push_back(lpg_Type);
            list.push_back(lpg_VariableDeclaratorId);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_VariableModifiersopt != nullptr) lpg_VariableModifiersopt->accept(v);
                lpg_Type->accept(v);
                lpg_VariableDeclaratorId->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 136:  VariableModifiers ::= VariableModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 137:  VariableModifiers ::= VariableModifiers VariableModifier
     *</b>
     */
    struct VariableModifiers :public Ast
    {
        IAst* lpg_VariableModifiers;
        IAst* lpg_VariableModifier;

        IAst* getVariableModifiers() { return lpg_VariableModifiers; };
        void setVariableModifiers(IAst* lpg_VariableModifiers) { this->lpg_VariableModifiers = lpg_VariableModifiers; }
        IAst* getVariableModifier() { return lpg_VariableModifier; };
        void setVariableModifier(IAst* lpg_VariableModifier) { this->lpg_VariableModifier = lpg_VariableModifier; }

        VariableModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableModifiers,
            IAst* lpg_VariableModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableModifiers = lpg_VariableModifiers;
            ((Ast*)lpg_VariableModifiers)->setParent(this);
            this->lpg_VariableModifier = lpg_VariableModifier;
            ((Ast*)lpg_VariableModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableModifiers);
            list.push_back(lpg_VariableModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_VariableModifiers->accept(v);
                lpg_VariableModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 139:  VariableModifier ::= Annotations
     *</em>
     *<p>
     *<b>
     *<li>Rule 138:  VariableModifier ::= final
     *</b>
     */
    struct VariableModifier :public AstToken
    {
        IToken* getfinal() { return leftIToken; }

        VariableModifier(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 140:  LastFormalParameter ::= VariableModifiersopt Type ...opt VariableDeclaratorId
     *</b>
     */
    struct LastFormalParameter :public Ast
    {
        IAst* lpg_VariableModifiersopt;
        IAst* lpg_Type;
        IAst* lpg_Ellipsisopt;
        IAst* lpg_VariableDeclaratorId;

        /**
         * The value returned by <b>getVariableModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getVariableModifiersopt() { return lpg_VariableModifiersopt; };
        void setVariableModifiersopt(IAst* lpg_VariableModifiersopt) { this->lpg_VariableModifiersopt = lpg_VariableModifiersopt; }
        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        /**
         * The value returned by <b>getEllipsisopt</b> may be <b>nullptr</b>
         */
        IAst* getEllipsisopt() { return lpg_Ellipsisopt; };
        void setEllipsisopt(IAst* lpg_Ellipsisopt) { this->lpg_Ellipsisopt = lpg_Ellipsisopt; }
        IAst* getVariableDeclaratorId() { return lpg_VariableDeclaratorId; };
        void setVariableDeclaratorId(IAst* lpg_VariableDeclaratorId) { this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId; }

        LastFormalParameter(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableModifiersopt,
            IAst* lpg_Type,
            IAst* lpg_Ellipsisopt,
            IAst* lpg_VariableDeclaratorId) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableModifiersopt = lpg_VariableModifiersopt;
            if (lpg_VariableModifiersopt != nullptr) ((Ast*)lpg_VariableModifiersopt)->setParent(this);
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_Ellipsisopt = lpg_Ellipsisopt;
            if (lpg_Ellipsisopt != nullptr) ((Ast*)lpg_Ellipsisopt)->setParent(this);
            this->lpg_VariableDeclaratorId = lpg_VariableDeclaratorId;
            ((Ast*)lpg_VariableDeclaratorId)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableModifiersopt);
            list.push_back(lpg_Type);
            list.push_back(lpg_Ellipsisopt);
            list.push_back(lpg_VariableDeclaratorId);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_VariableModifiersopt != nullptr) lpg_VariableModifiersopt->accept(v);
                lpg_Type->accept(v);
                if (lpg_Ellipsisopt != nullptr) lpg_Ellipsisopt->accept(v);
                lpg_VariableDeclaratorId->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 141:  MethodModifiers ::= MethodModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 142:  MethodModifiers ::= MethodModifiers MethodModifier
     *</b>
     */
    struct MethodModifiers :public Ast
    {
        IAst* lpg_MethodModifiers;
        IAst* lpg_MethodModifier;

        IAst* getMethodModifiers() { return lpg_MethodModifiers; };
        void setMethodModifiers(IAst* lpg_MethodModifiers) { this->lpg_MethodModifiers = lpg_MethodModifiers; }
        IAst* getMethodModifier() { return lpg_MethodModifier; };
        void setMethodModifier(IAst* lpg_MethodModifier) { this->lpg_MethodModifier = lpg_MethodModifier; }

        MethodModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MethodModifiers,
            IAst* lpg_MethodModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_MethodModifiers = lpg_MethodModifiers;
            ((Ast*)lpg_MethodModifiers)->setParent(this);
            this->lpg_MethodModifier = lpg_MethodModifier;
            ((Ast*)lpg_MethodModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MethodModifiers);
            list.push_back(lpg_MethodModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MethodModifiers->accept(v);
                lpg_MethodModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 153:  Throws ::= throws ExceptionTypeList
     *</b>
     */
    struct Throws :public Ast
    {
        IAst* lpg_throws;
        IAst* lpg_ExceptionTypeList;

        IAst* getthrows() { return lpg_throws; };
        void setthrows(IAst* lpg_throws) { this->lpg_throws = lpg_throws; }
        IAst* getExceptionTypeList() { return lpg_ExceptionTypeList; };
        void setExceptionTypeList(IAst* lpg_ExceptionTypeList) { this->lpg_ExceptionTypeList = lpg_ExceptionTypeList; }

        Throws(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_throws,
            IAst* lpg_ExceptionTypeList) :Ast(leftIToken, rightIToken) {
            this->lpg_throws = lpg_throws;
            ((Ast*)lpg_throws)->setParent(this);
            this->lpg_ExceptionTypeList = lpg_ExceptionTypeList;
            ((Ast*)lpg_ExceptionTypeList)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_throws);
            list.push_back(lpg_ExceptionTypeList);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_throws->accept(v);
                lpg_ExceptionTypeList->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 154:  ExceptionTypeList ::= ExceptionType
     *</em>
     *<p>
     *<b>
     *<li>Rule 155:  ExceptionTypeList ::= ExceptionTypeList , ExceptionType
     *</b>
     */
    struct ExceptionTypeList :public Ast
    {
        IAst* lpg_ExceptionTypeList;
        IAst* lpg_COMMA;
        IAst* lpg_ExceptionType;

        IAst* getExceptionTypeList() { return lpg_ExceptionTypeList; };
        void setExceptionTypeList(IAst* lpg_ExceptionTypeList) { this->lpg_ExceptionTypeList = lpg_ExceptionTypeList; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getExceptionType() { return lpg_ExceptionType; };
        void setExceptionType(IAst* lpg_ExceptionType) { this->lpg_ExceptionType = lpg_ExceptionType; }

        ExceptionTypeList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ExceptionTypeList,
            IAst* lpg_COMMA,
            IAst* lpg_ExceptionType) :Ast(leftIToken, rightIToken) {
            this->lpg_ExceptionTypeList = lpg_ExceptionTypeList;
            ((Ast*)lpg_ExceptionTypeList)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_ExceptionType = lpg_ExceptionType;
            ((Ast*)lpg_ExceptionType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ExceptionTypeList);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_ExceptionType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ExceptionTypeList->accept(v);
                lpg_COMMA->accept(v);
                lpg_ExceptionType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 158:  MethodBody ::= Block
     *</em>
     *<p>
     *<b>
     *<li>Rule 159:  MethodBody ::= ;
     *</b>
     */
    struct MethodBody :public AstToken
    {
        IToken* getSEMICOLON() { return leftIToken; }

        MethodBody(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 161:  StaticInitializer ::= static Block
     *</b>
     */
    struct StaticInitializer :public Ast
    {
        IAst* lpg_static;
        IAst* lpg_Block;

        IAst* getstatic() { return lpg_static; };
        void setstatic(IAst* lpg_static) { this->lpg_static = lpg_static; }
        IAst* getBlock() { return lpg_Block; };
        void setBlock(IAst* lpg_Block) { this->lpg_Block = lpg_Block; }

        StaticInitializer(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_static,
            IAst* lpg_Block) :Ast(leftIToken, rightIToken) {
            this->lpg_static = lpg_static;
            ((Ast*)lpg_static)->setParent(this);
            this->lpg_Block = lpg_Block;
            ((Ast*)lpg_Block)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_static);
            list.push_back(lpg_Block);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_static->accept(v);
                lpg_Block->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 162:  ConstructorDeclaration ::= ConstructorModifiersopt ConstructorDeclarator Throwsopt ConstructorBody
     *</b>
     */
    struct ConstructorDeclaration :public Ast
    {
        IAst* lpg_ConstructorModifiersopt;
        IAst* lpg_ConstructorDeclarator;
        IAst* lpg_Throwsopt;
        IAst* lpg_ConstructorBody;

        /**
         * The value returned by <b>getConstructorModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getConstructorModifiersopt() { return lpg_ConstructorModifiersopt; };
        void setConstructorModifiersopt(IAst* lpg_ConstructorModifiersopt) { this->lpg_ConstructorModifiersopt = lpg_ConstructorModifiersopt; }
        IAst* getConstructorDeclarator() { return lpg_ConstructorDeclarator; };
        void setConstructorDeclarator(IAst* lpg_ConstructorDeclarator) { this->lpg_ConstructorDeclarator = lpg_ConstructorDeclarator; }
        /**
         * The value returned by <b>getThrowsopt</b> may be <b>nullptr</b>
         */
        IAst* getThrowsopt() { return lpg_Throwsopt; };
        void setThrowsopt(IAst* lpg_Throwsopt) { this->lpg_Throwsopt = lpg_Throwsopt; }
        IAst* getConstructorBody() { return lpg_ConstructorBody; };
        void setConstructorBody(IAst* lpg_ConstructorBody) { this->lpg_ConstructorBody = lpg_ConstructorBody; }

        ConstructorDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConstructorModifiersopt,
            IAst* lpg_ConstructorDeclarator,
            IAst* lpg_Throwsopt,
            IAst* lpg_ConstructorBody) :Ast(leftIToken, rightIToken) {
            this->lpg_ConstructorModifiersopt = lpg_ConstructorModifiersopt;
            if (lpg_ConstructorModifiersopt != nullptr) ((Ast*)lpg_ConstructorModifiersopt)->setParent(this);
            this->lpg_ConstructorDeclarator = lpg_ConstructorDeclarator;
            ((Ast*)lpg_ConstructorDeclarator)->setParent(this);
            this->lpg_Throwsopt = lpg_Throwsopt;
            if (lpg_Throwsopt != nullptr) ((Ast*)lpg_Throwsopt)->setParent(this);
            this->lpg_ConstructorBody = lpg_ConstructorBody;
            ((Ast*)lpg_ConstructorBody)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConstructorModifiersopt);
            list.push_back(lpg_ConstructorDeclarator);
            list.push_back(lpg_Throwsopt);
            list.push_back(lpg_ConstructorBody);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_ConstructorModifiersopt != nullptr) lpg_ConstructorModifiersopt->accept(v);
                lpg_ConstructorDeclarator->accept(v);
                if (lpg_Throwsopt != nullptr) lpg_Throwsopt->accept(v);
                lpg_ConstructorBody->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 163:  ConstructorDeclarator ::= TypeParametersopt SimpleTypeName ( FormalParameterListopt )
     *</b>
     */
    struct ConstructorDeclarator :public Ast
    {
        IAst* lpg_TypeParametersopt;
        IAst* lpg_SimpleTypeName;
        IAst* lpg_LPAREN;
        IAst* lpg_FormalParameterListopt;
        IAst* lpg_RPAREN;

        /**
         * The value returned by <b>getTypeParametersopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeParametersopt() { return lpg_TypeParametersopt; };
        void setTypeParametersopt(IAst* lpg_TypeParametersopt) { this->lpg_TypeParametersopt = lpg_TypeParametersopt; }
        IAst* getSimpleTypeName() { return lpg_SimpleTypeName; };
        void setSimpleTypeName(IAst* lpg_SimpleTypeName) { this->lpg_SimpleTypeName = lpg_SimpleTypeName; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getFormalParameterListopt</b> may be <b>nullptr</b>
         */
        IAst* getFormalParameterListopt() { return lpg_FormalParameterListopt; };
        void setFormalParameterListopt(IAst* lpg_FormalParameterListopt) { this->lpg_FormalParameterListopt = lpg_FormalParameterListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        ConstructorDeclarator(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeParametersopt,
            IAst* lpg_SimpleTypeName,
            IAst* lpg_LPAREN,
            IAst* lpg_FormalParameterListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeParametersopt = lpg_TypeParametersopt;
            if (lpg_TypeParametersopt != nullptr) ((Ast*)lpg_TypeParametersopt)->setParent(this);
            this->lpg_SimpleTypeName = lpg_SimpleTypeName;
            ((Ast*)lpg_SimpleTypeName)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_FormalParameterListopt = lpg_FormalParameterListopt;
            if (lpg_FormalParameterListopt != nullptr) ((Ast*)lpg_FormalParameterListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeParametersopt);
            list.push_back(lpg_SimpleTypeName);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_FormalParameterListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_TypeParametersopt != nullptr) lpg_TypeParametersopt->accept(v);
                lpg_SimpleTypeName->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_FormalParameterListopt != nullptr) lpg_FormalParameterListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 165:  ConstructorModifiers ::= ConstructorModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 166:  ConstructorModifiers ::= ConstructorModifiers ConstructorModifier
     *</b>
     */
    struct ConstructorModifiers :public Ast
    {
        IAst* lpg_ConstructorModifiers;
        IAst* lpg_ConstructorModifier;

        IAst* getConstructorModifiers() { return lpg_ConstructorModifiers; };
        void setConstructorModifiers(IAst* lpg_ConstructorModifiers) { this->lpg_ConstructorModifiers = lpg_ConstructorModifiers; }
        IAst* getConstructorModifier() { return lpg_ConstructorModifier; };
        void setConstructorModifier(IAst* lpg_ConstructorModifier) { this->lpg_ConstructorModifier = lpg_ConstructorModifier; }

        ConstructorModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConstructorModifiers,
            IAst* lpg_ConstructorModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_ConstructorModifiers = lpg_ConstructorModifiers;
            ((Ast*)lpg_ConstructorModifiers)->setParent(this);
            this->lpg_ConstructorModifier = lpg_ConstructorModifier;
            ((Ast*)lpg_ConstructorModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConstructorModifiers);
            list.push_back(lpg_ConstructorModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ConstructorModifiers->accept(v);
                lpg_ConstructorModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 171:  ConstructorBody ::= { ExplicitConstructorInvocationopt BlockStatementsopt }
     *</b>
     */
    struct ConstructorBody :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_ExplicitConstructorInvocationopt;
        IAst* lpg_BlockStatementsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getExplicitConstructorInvocationopt</b> may be <b>nullptr</b>
         */
        IAst* getExplicitConstructorInvocationopt() { return lpg_ExplicitConstructorInvocationopt; };
        void setExplicitConstructorInvocationopt(IAst* lpg_ExplicitConstructorInvocationopt) { this->lpg_ExplicitConstructorInvocationopt = lpg_ExplicitConstructorInvocationopt; }
        /**
         * The value returned by <b>getBlockStatementsopt</b> may be <b>nullptr</b>
         */
        IAst* getBlockStatementsopt() { return lpg_BlockStatementsopt; };
        void setBlockStatementsopt(IAst* lpg_BlockStatementsopt) { this->lpg_BlockStatementsopt = lpg_BlockStatementsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        ConstructorBody(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_ExplicitConstructorInvocationopt,
            IAst* lpg_BlockStatementsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_ExplicitConstructorInvocationopt = lpg_ExplicitConstructorInvocationopt;
            if (lpg_ExplicitConstructorInvocationopt != nullptr) ((Ast*)lpg_ExplicitConstructorInvocationopt)->setParent(this);
            this->lpg_BlockStatementsopt = lpg_BlockStatementsopt;
            if (lpg_BlockStatementsopt != nullptr) ((Ast*)lpg_BlockStatementsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_ExplicitConstructorInvocationopt);
            list.push_back(lpg_BlockStatementsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_ExplicitConstructorInvocationopt != nullptr) lpg_ExplicitConstructorInvocationopt->accept(v);
                if (lpg_BlockStatementsopt != nullptr) lpg_BlockStatementsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 175:  EnumDeclaration ::= ClassModifiersopt enum identifier Interfacesopt EnumBody
     *</b>
     */
    struct EnumDeclaration :public Ast
    {
        IAst* lpg_ClassModifiersopt;
        IAst* lpg_enum;
        IAst* lpg_identifier;
        IAst* lpg_Interfacesopt;
        IAst* lpg_EnumBody;

        /**
         * The value returned by <b>getClassModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getClassModifiersopt() { return lpg_ClassModifiersopt; };
        void setClassModifiersopt(IAst* lpg_ClassModifiersopt) { this->lpg_ClassModifiersopt = lpg_ClassModifiersopt; }
        IAst* getenum() { return lpg_enum; };
        void setenum(IAst* lpg_enum) { this->lpg_enum = lpg_enum; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        /**
         * The value returned by <b>getInterfacesopt</b> may be <b>nullptr</b>
         */
        IAst* getInterfacesopt() { return lpg_Interfacesopt; };
        void setInterfacesopt(IAst* lpg_Interfacesopt) { this->lpg_Interfacesopt = lpg_Interfacesopt; }
        IAst* getEnumBody() { return lpg_EnumBody; };
        void setEnumBody(IAst* lpg_EnumBody) { this->lpg_EnumBody = lpg_EnumBody; }

        EnumDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassModifiersopt,
            IAst* lpg_enum,
            IAst* lpg_identifier,
            IAst* lpg_Interfacesopt,
            IAst* lpg_EnumBody) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassModifiersopt = lpg_ClassModifiersopt;
            if (lpg_ClassModifiersopt != nullptr) ((Ast*)lpg_ClassModifiersopt)->setParent(this);
            this->lpg_enum = lpg_enum;
            ((Ast*)lpg_enum)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_Interfacesopt = lpg_Interfacesopt;
            if (lpg_Interfacesopt != nullptr) ((Ast*)lpg_Interfacesopt)->setParent(this);
            this->lpg_EnumBody = lpg_EnumBody;
            ((Ast*)lpg_EnumBody)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassModifiersopt);
            list.push_back(lpg_enum);
            list.push_back(lpg_identifier);
            list.push_back(lpg_Interfacesopt);
            list.push_back(lpg_EnumBody);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_ClassModifiersopt != nullptr) lpg_ClassModifiersopt->accept(v);
                lpg_enum->accept(v);
                lpg_identifier->accept(v);
                if (lpg_Interfacesopt != nullptr) lpg_Interfacesopt->accept(v);
                lpg_EnumBody->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 176:  EnumBody ::= { EnumConstantsopt ,opt EnumBodyDeclarationsopt }
     *</b>
     */
    struct EnumBody :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_EnumConstantsopt;
        IAst* lpg_Commaopt;
        IAst* lpg_EnumBodyDeclarationsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getEnumConstantsopt</b> may be <b>nullptr</b>
         */
        IAst* getEnumConstantsopt() { return lpg_EnumConstantsopt; };
        void setEnumConstantsopt(IAst* lpg_EnumConstantsopt) { this->lpg_EnumConstantsopt = lpg_EnumConstantsopt; }
        /**
         * The value returned by <b>getCommaopt</b> may be <b>nullptr</b>
         */
        IAst* getCommaopt() { return lpg_Commaopt; };
        void setCommaopt(IAst* lpg_Commaopt) { this->lpg_Commaopt = lpg_Commaopt; }
        /**
         * The value returned by <b>getEnumBodyDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getEnumBodyDeclarationsopt() { return lpg_EnumBodyDeclarationsopt; };
        void setEnumBodyDeclarationsopt(IAst* lpg_EnumBodyDeclarationsopt) { this->lpg_EnumBodyDeclarationsopt = lpg_EnumBodyDeclarationsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        EnumBody(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_EnumConstantsopt,
            IAst* lpg_Commaopt,
            IAst* lpg_EnumBodyDeclarationsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_EnumConstantsopt = lpg_EnumConstantsopt;
            if (lpg_EnumConstantsopt != nullptr) ((Ast*)lpg_EnumConstantsopt)->setParent(this);
            this->lpg_Commaopt = lpg_Commaopt;
            if (lpg_Commaopt != nullptr) ((Ast*)lpg_Commaopt)->setParent(this);
            this->lpg_EnumBodyDeclarationsopt = lpg_EnumBodyDeclarationsopt;
            if (lpg_EnumBodyDeclarationsopt != nullptr) ((Ast*)lpg_EnumBodyDeclarationsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_EnumConstantsopt);
            list.push_back(lpg_Commaopt);
            list.push_back(lpg_EnumBodyDeclarationsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_EnumConstantsopt != nullptr) lpg_EnumConstantsopt->accept(v);
                if (lpg_Commaopt != nullptr) lpg_Commaopt->accept(v);
                if (lpg_EnumBodyDeclarationsopt != nullptr) lpg_EnumBodyDeclarationsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 177:  EnumConstants ::= EnumConstant
     *</em>
     *<p>
     *<b>
     *<li>Rule 178:  EnumConstants ::= EnumConstants , EnumConstant
     *</b>
     */
    struct EnumConstants :public Ast
    {
        IAst* lpg_EnumConstants;
        IAst* lpg_COMMA;
        IAst* lpg_EnumConstant;

        IAst* getEnumConstants() { return lpg_EnumConstants; };
        void setEnumConstants(IAst* lpg_EnumConstants) { this->lpg_EnumConstants = lpg_EnumConstants; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getEnumConstant() { return lpg_EnumConstant; };
        void setEnumConstant(IAst* lpg_EnumConstant) { this->lpg_EnumConstant = lpg_EnumConstant; }

        EnumConstants(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_EnumConstants,
            IAst* lpg_COMMA,
            IAst* lpg_EnumConstant) :Ast(leftIToken, rightIToken) {
            this->lpg_EnumConstants = lpg_EnumConstants;
            ((Ast*)lpg_EnumConstants)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_EnumConstant = lpg_EnumConstant;
            ((Ast*)lpg_EnumConstant)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_EnumConstants);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_EnumConstant);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_EnumConstants->accept(v);
                lpg_COMMA->accept(v);
                lpg_EnumConstant->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 307:  EnumConstant ::= identifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 179:  EnumConstant ::= Annotationsopt identifier Argumentsopt ClassBodyopt
     *</b>
     */
    struct EnumConstant :public Ast
    {
        IAst* lpg_Annotationsopt;
        IAst* lpg_identifier;
        IAst* lpg_Argumentsopt;
        IAst* lpg_ClassBodyopt;

        /**
         * The value returned by <b>getAnnotationsopt</b> may be <b>nullptr</b>
         */
        IAst* getAnnotationsopt() { return lpg_Annotationsopt; };
        void setAnnotationsopt(IAst* lpg_Annotationsopt) { this->lpg_Annotationsopt = lpg_Annotationsopt; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        /**
         * The value returned by <b>getArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentsopt() { return lpg_Argumentsopt; };
        void setArgumentsopt(IAst* lpg_Argumentsopt) { this->lpg_Argumentsopt = lpg_Argumentsopt; }
        /**
         * The value returned by <b>getClassBodyopt</b> may be <b>nullptr</b>
         */
        IAst* getClassBodyopt() { return lpg_ClassBodyopt; };
        void setClassBodyopt(IAst* lpg_ClassBodyopt) { this->lpg_ClassBodyopt = lpg_ClassBodyopt; }

        EnumConstant(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Annotationsopt,
            IAst* lpg_identifier,
            IAst* lpg_Argumentsopt,
            IAst* lpg_ClassBodyopt) :Ast(leftIToken, rightIToken) {
            this->lpg_Annotationsopt = lpg_Annotationsopt;
            if (lpg_Annotationsopt != nullptr) ((Ast*)lpg_Annotationsopt)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_Argumentsopt = lpg_Argumentsopt;
            if (lpg_Argumentsopt != nullptr) ((Ast*)lpg_Argumentsopt)->setParent(this);
            this->lpg_ClassBodyopt = lpg_ClassBodyopt;
            if (lpg_ClassBodyopt != nullptr) ((Ast*)lpg_ClassBodyopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Annotationsopt);
            list.push_back(lpg_identifier);
            list.push_back(lpg_Argumentsopt);
            list.push_back(lpg_ClassBodyopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_Annotationsopt != nullptr) lpg_Annotationsopt->accept(v);
                lpg_identifier->accept(v);
                if (lpg_Argumentsopt != nullptr) lpg_Argumentsopt->accept(v);
                if (lpg_ClassBodyopt != nullptr) lpg_ClassBodyopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 180:  Arguments ::= ( ArgumentListopt )
     *</b>
     */
    struct Arguments :public Ast
    {
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;

        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        Arguments(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 181:  EnumBodyDeclarations ::= ; ClassBodyDeclarationsopt
     *</b>
     */
    struct EnumBodyDeclarations :public Ast
    {
        IAst* lpg_SEMICOLON;
        IAst* lpg_ClassBodyDeclarationsopt;

        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }
        /**
         * The value returned by <b>getClassBodyDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getClassBodyDeclarationsopt() { return lpg_ClassBodyDeclarationsopt; };
        void setClassBodyDeclarationsopt(IAst* lpg_ClassBodyDeclarationsopt) { this->lpg_ClassBodyDeclarationsopt = lpg_ClassBodyDeclarationsopt; }

        EnumBodyDeclarations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_SEMICOLON,
            IAst* lpg_ClassBodyDeclarationsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            this->lpg_ClassBodyDeclarationsopt = lpg_ClassBodyDeclarationsopt;
            if (lpg_ClassBodyDeclarationsopt != nullptr) ((Ast*)lpg_ClassBodyDeclarationsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_SEMICOLON);
            list.push_back(lpg_ClassBodyDeclarationsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_SEMICOLON->accept(v);
                if (lpg_ClassBodyDeclarationsopt != nullptr) lpg_ClassBodyDeclarationsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 184:  NormalInterfaceDeclaration ::= InterfaceModifiersopt interface identifier TypeParametersopt ExtendsInterfacesopt InterfaceBody
     *</b>
     */
    struct NormalInterfaceDeclaration :public Ast
    {
        IAst* lpg_InterfaceModifiersopt;
        IAst* lpg_interface;
        IAst* lpg_identifier;
        IAst* lpg_TypeParametersopt;
        IAst* lpg_ExtendsInterfacesopt;
        IAst* lpg_InterfaceBody;

        /**
         * The value returned by <b>getInterfaceModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getInterfaceModifiersopt() { return lpg_InterfaceModifiersopt; };
        void setInterfaceModifiersopt(IAst* lpg_InterfaceModifiersopt) { this->lpg_InterfaceModifiersopt = lpg_InterfaceModifiersopt; }
        IAst* getinterface() { return lpg_interface; };
        void setinterface(IAst* lpg_interface) { this->lpg_interface = lpg_interface; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        /**
         * The value returned by <b>getTypeParametersopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeParametersopt() { return lpg_TypeParametersopt; };
        void setTypeParametersopt(IAst* lpg_TypeParametersopt) { this->lpg_TypeParametersopt = lpg_TypeParametersopt; }
        /**
         * The value returned by <b>getExtendsInterfacesopt</b> may be <b>nullptr</b>
         */
        IAst* getExtendsInterfacesopt() { return lpg_ExtendsInterfacesopt; };
        void setExtendsInterfacesopt(IAst* lpg_ExtendsInterfacesopt) { this->lpg_ExtendsInterfacesopt = lpg_ExtendsInterfacesopt; }
        IAst* getInterfaceBody() { return lpg_InterfaceBody; };
        void setInterfaceBody(IAst* lpg_InterfaceBody) { this->lpg_InterfaceBody = lpg_InterfaceBody; }

        NormalInterfaceDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_InterfaceModifiersopt,
            IAst* lpg_interface,
            IAst* lpg_identifier,
            IAst* lpg_TypeParametersopt,
            IAst* lpg_ExtendsInterfacesopt,
            IAst* lpg_InterfaceBody) :Ast(leftIToken, rightIToken) {
            this->lpg_InterfaceModifiersopt = lpg_InterfaceModifiersopt;
            if (lpg_InterfaceModifiersopt != nullptr) ((Ast*)lpg_InterfaceModifiersopt)->setParent(this);
            this->lpg_interface = lpg_interface;
            ((Ast*)lpg_interface)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_TypeParametersopt = lpg_TypeParametersopt;
            if (lpg_TypeParametersopt != nullptr) ((Ast*)lpg_TypeParametersopt)->setParent(this);
            this->lpg_ExtendsInterfacesopt = lpg_ExtendsInterfacesopt;
            if (lpg_ExtendsInterfacesopt != nullptr) ((Ast*)lpg_ExtendsInterfacesopt)->setParent(this);
            this->lpg_InterfaceBody = lpg_InterfaceBody;
            ((Ast*)lpg_InterfaceBody)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_InterfaceModifiersopt);
            list.push_back(lpg_interface);
            list.push_back(lpg_identifier);
            list.push_back(lpg_TypeParametersopt);
            list.push_back(lpg_ExtendsInterfacesopt);
            list.push_back(lpg_InterfaceBody);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_InterfaceModifiersopt != nullptr) lpg_InterfaceModifiersopt->accept(v);
                lpg_interface->accept(v);
                lpg_identifier->accept(v);
                if (lpg_TypeParametersopt != nullptr) lpg_TypeParametersopt->accept(v);
                if (lpg_ExtendsInterfacesopt != nullptr) lpg_ExtendsInterfacesopt->accept(v);
                lpg_InterfaceBody->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 185:  InterfaceModifiers ::= InterfaceModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 186:  InterfaceModifiers ::= InterfaceModifiers InterfaceModifier
     *</b>
     */
    struct InterfaceModifiers :public Ast
    {
        IAst* lpg_InterfaceModifiers;
        IAst* lpg_InterfaceModifier;

        IAst* getInterfaceModifiers() { return lpg_InterfaceModifiers; };
        void setInterfaceModifiers(IAst* lpg_InterfaceModifiers) { this->lpg_InterfaceModifiers = lpg_InterfaceModifiers; }
        IAst* getInterfaceModifier() { return lpg_InterfaceModifier; };
        void setInterfaceModifier(IAst* lpg_InterfaceModifier) { this->lpg_InterfaceModifier = lpg_InterfaceModifier; }

        InterfaceModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_InterfaceModifiers,
            IAst* lpg_InterfaceModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_InterfaceModifiers = lpg_InterfaceModifiers;
            ((Ast*)lpg_InterfaceModifiers)->setParent(this);
            this->lpg_InterfaceModifier = lpg_InterfaceModifier;
            ((Ast*)lpg_InterfaceModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_InterfaceModifiers);
            list.push_back(lpg_InterfaceModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_InterfaceModifiers->accept(v);
                lpg_InterfaceModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 196:  InterfaceBody ::= { InterfaceMemberDeclarationsopt }
     *</b>
     */
    struct InterfaceBody :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_InterfaceMemberDeclarationsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getInterfaceMemberDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getInterfaceMemberDeclarationsopt() { return lpg_InterfaceMemberDeclarationsopt; };
        void setInterfaceMemberDeclarationsopt(IAst* lpg_InterfaceMemberDeclarationsopt) { this->lpg_InterfaceMemberDeclarationsopt = lpg_InterfaceMemberDeclarationsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        InterfaceBody(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_InterfaceMemberDeclarationsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_InterfaceMemberDeclarationsopt = lpg_InterfaceMemberDeclarationsopt;
            if (lpg_InterfaceMemberDeclarationsopt != nullptr) ((Ast*)lpg_InterfaceMemberDeclarationsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_InterfaceMemberDeclarationsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_InterfaceMemberDeclarationsopt != nullptr) lpg_InterfaceMemberDeclarationsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 197:  InterfaceMemberDeclarations ::= InterfaceMemberDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 198:  InterfaceMemberDeclarations ::= InterfaceMemberDeclarations InterfaceMemberDeclaration
     *</b>
     */
    struct InterfaceMemberDeclarations :public Ast
    {
        IAst* lpg_InterfaceMemberDeclarations;
        IAst* lpg_InterfaceMemberDeclaration;

        IAst* getInterfaceMemberDeclarations() { return lpg_InterfaceMemberDeclarations; };
        void setInterfaceMemberDeclarations(IAst* lpg_InterfaceMemberDeclarations) { this->lpg_InterfaceMemberDeclarations = lpg_InterfaceMemberDeclarations; }
        IAst* getInterfaceMemberDeclaration() { return lpg_InterfaceMemberDeclaration; };
        void setInterfaceMemberDeclaration(IAst* lpg_InterfaceMemberDeclaration) { this->lpg_InterfaceMemberDeclaration = lpg_InterfaceMemberDeclaration; }

        InterfaceMemberDeclarations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_InterfaceMemberDeclarations,
            IAst* lpg_InterfaceMemberDeclaration) :Ast(leftIToken, rightIToken) {
            this->lpg_InterfaceMemberDeclarations = lpg_InterfaceMemberDeclarations;
            ((Ast*)lpg_InterfaceMemberDeclarations)->setParent(this);
            this->lpg_InterfaceMemberDeclaration = lpg_InterfaceMemberDeclaration;
            ((Ast*)lpg_InterfaceMemberDeclaration)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_InterfaceMemberDeclarations);
            list.push_back(lpg_InterfaceMemberDeclaration);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_InterfaceMemberDeclarations->accept(v);
                lpg_InterfaceMemberDeclaration->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 199:  InterfaceMemberDeclaration ::= ConstantDeclaration
     *<li>Rule 200:  InterfaceMemberDeclaration ::= AbstractMethodDeclaration
     *<li>Rule 201:  InterfaceMemberDeclaration ::= ClassDeclaration
     *<li>Rule 202:  InterfaceMemberDeclaration ::= InterfaceDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 203:  InterfaceMemberDeclaration ::= ;
     *</b>
     */
    struct InterfaceMemberDeclaration :public AstToken
    {
        IToken* getSEMICOLON() { return leftIToken; }

        InterfaceMemberDeclaration(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 204:  ConstantDeclaration ::= ConstantModifiersopt Type VariableDeclarators
     *</b>
     */
    struct ConstantDeclaration :public Ast
    {
        IAst* lpg_ConstantModifiersopt;
        IAst* lpg_Type;
        IAst* lpg_VariableDeclarators;

        /**
         * The value returned by <b>getConstantModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getConstantModifiersopt() { return lpg_ConstantModifiersopt; };
        void setConstantModifiersopt(IAst* lpg_ConstantModifiersopt) { this->lpg_ConstantModifiersopt = lpg_ConstantModifiersopt; }
        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getVariableDeclarators() { return lpg_VariableDeclarators; };
        void setVariableDeclarators(IAst* lpg_VariableDeclarators) { this->lpg_VariableDeclarators = lpg_VariableDeclarators; }

        ConstantDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConstantModifiersopt,
            IAst* lpg_Type,
            IAst* lpg_VariableDeclarators) :Ast(leftIToken, rightIToken) {
            this->lpg_ConstantModifiersopt = lpg_ConstantModifiersopt;
            if (lpg_ConstantModifiersopt != nullptr) ((Ast*)lpg_ConstantModifiersopt)->setParent(this);
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_VariableDeclarators = lpg_VariableDeclarators;
            ((Ast*)lpg_VariableDeclarators)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConstantModifiersopt);
            list.push_back(lpg_Type);
            list.push_back(lpg_VariableDeclarators);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_ConstantModifiersopt != nullptr) lpg_ConstantModifiersopt->accept(v);
                lpg_Type->accept(v);
                lpg_VariableDeclarators->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 205:  ConstantModifiers ::= ConstantModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 206:  ConstantModifiers ::= ConstantModifiers ConstantModifier
     *</b>
     */
    struct ConstantModifiers :public Ast
    {
        IAst* lpg_ConstantModifiers;
        IAst* lpg_ConstantModifier;

        IAst* getConstantModifiers() { return lpg_ConstantModifiers; };
        void setConstantModifiers(IAst* lpg_ConstantModifiers) { this->lpg_ConstantModifiers = lpg_ConstantModifiers; }
        IAst* getConstantModifier() { return lpg_ConstantModifier; };
        void setConstantModifier(IAst* lpg_ConstantModifier) { this->lpg_ConstantModifier = lpg_ConstantModifier; }

        ConstantModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConstantModifiers,
            IAst* lpg_ConstantModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_ConstantModifiers = lpg_ConstantModifiers;
            ((Ast*)lpg_ConstantModifiers)->setParent(this);
            this->lpg_ConstantModifier = lpg_ConstantModifier;
            ((Ast*)lpg_ConstantModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConstantModifiers);
            list.push_back(lpg_ConstantModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ConstantModifiers->accept(v);
                lpg_ConstantModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 211:  AbstractMethodDeclaration ::= AbstractMethodModifiersopt TypeParametersopt ResultType MethodDeclarator Throwsopt ;
     *</b>
     */
    struct AbstractMethodDeclaration :public Ast
    {
        IAst* lpg_AbstractMethodModifiersopt;
        IAst* lpg_TypeParametersopt;
        IAst* lpg_ResultType;
        IAst* lpg_MethodDeclarator;
        IAst* lpg_Throwsopt;
        IAst* lpg_SEMICOLON;

        /**
         * The value returned by <b>getAbstractMethodModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getAbstractMethodModifiersopt() { return lpg_AbstractMethodModifiersopt; };
        void setAbstractMethodModifiersopt(IAst* lpg_AbstractMethodModifiersopt) { this->lpg_AbstractMethodModifiersopt = lpg_AbstractMethodModifiersopt; }
        /**
         * The value returned by <b>getTypeParametersopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeParametersopt() { return lpg_TypeParametersopt; };
        void setTypeParametersopt(IAst* lpg_TypeParametersopt) { this->lpg_TypeParametersopt = lpg_TypeParametersopt; }
        IAst* getResultType() { return lpg_ResultType; };
        void setResultType(IAst* lpg_ResultType) { this->lpg_ResultType = lpg_ResultType; }
        IAst* getMethodDeclarator() { return lpg_MethodDeclarator; };
        void setMethodDeclarator(IAst* lpg_MethodDeclarator) { this->lpg_MethodDeclarator = lpg_MethodDeclarator; }
        /**
         * The value returned by <b>getThrowsopt</b> may be <b>nullptr</b>
         */
        IAst* getThrowsopt() { return lpg_Throwsopt; };
        void setThrowsopt(IAst* lpg_Throwsopt) { this->lpg_Throwsopt = lpg_Throwsopt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        AbstractMethodDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AbstractMethodModifiersopt,
            IAst* lpg_TypeParametersopt,
            IAst* lpg_ResultType,
            IAst* lpg_MethodDeclarator,
            IAst* lpg_Throwsopt,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_AbstractMethodModifiersopt = lpg_AbstractMethodModifiersopt;
            if (lpg_AbstractMethodModifiersopt != nullptr) ((Ast*)lpg_AbstractMethodModifiersopt)->setParent(this);
            this->lpg_TypeParametersopt = lpg_TypeParametersopt;
            if (lpg_TypeParametersopt != nullptr) ((Ast*)lpg_TypeParametersopt)->setParent(this);
            this->lpg_ResultType = lpg_ResultType;
            ((Ast*)lpg_ResultType)->setParent(this);
            this->lpg_MethodDeclarator = lpg_MethodDeclarator;
            ((Ast*)lpg_MethodDeclarator)->setParent(this);
            this->lpg_Throwsopt = lpg_Throwsopt;
            if (lpg_Throwsopt != nullptr) ((Ast*)lpg_Throwsopt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AbstractMethodModifiersopt);
            list.push_back(lpg_TypeParametersopt);
            list.push_back(lpg_ResultType);
            list.push_back(lpg_MethodDeclarator);
            list.push_back(lpg_Throwsopt);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_AbstractMethodModifiersopt != nullptr) lpg_AbstractMethodModifiersopt->accept(v);
                if (lpg_TypeParametersopt != nullptr) lpg_TypeParametersopt->accept(v);
                lpg_ResultType->accept(v);
                lpg_MethodDeclarator->accept(v);
                if (lpg_Throwsopt != nullptr) lpg_Throwsopt->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 212:  AbstractMethodModifiers ::= AbstractMethodModifier
     *</em>
     *<p>
     *<b>
     *<li>Rule 213:  AbstractMethodModifiers ::= AbstractMethodModifiers AbstractMethodModifier
     *</b>
     */
    struct AbstractMethodModifiers :public Ast
    {
        IAst* lpg_AbstractMethodModifiers;
        IAst* lpg_AbstractMethodModifier;

        IAst* getAbstractMethodModifiers() { return lpg_AbstractMethodModifiers; };
        void setAbstractMethodModifiers(IAst* lpg_AbstractMethodModifiers) { this->lpg_AbstractMethodModifiers = lpg_AbstractMethodModifiers; }
        IAst* getAbstractMethodModifier() { return lpg_AbstractMethodModifier; };
        void setAbstractMethodModifier(IAst* lpg_AbstractMethodModifier) { this->lpg_AbstractMethodModifier = lpg_AbstractMethodModifier; }

        AbstractMethodModifiers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AbstractMethodModifiers,
            IAst* lpg_AbstractMethodModifier) :Ast(leftIToken, rightIToken) {
            this->lpg_AbstractMethodModifiers = lpg_AbstractMethodModifiers;
            ((Ast*)lpg_AbstractMethodModifiers)->setParent(this);
            this->lpg_AbstractMethodModifier = lpg_AbstractMethodModifier;
            ((Ast*)lpg_AbstractMethodModifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AbstractMethodModifiers);
            list.push_back(lpg_AbstractMethodModifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AbstractMethodModifiers->accept(v);
                lpg_AbstractMethodModifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 217:  AnnotationTypeDeclaration ::= InterfaceModifiersopt @ interface identifier AnnotationTypeBody
     *</b>
     */
    struct AnnotationTypeDeclaration :public Ast
    {
        IAst* lpg_InterfaceModifiersopt;
        IAst* lpg_AT;
        IAst* lpg_interface;
        IAst* lpg_identifier;
        IAst* lpg_AnnotationTypeBody;

        /**
         * The value returned by <b>getInterfaceModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getInterfaceModifiersopt() { return lpg_InterfaceModifiersopt; };
        void setInterfaceModifiersopt(IAst* lpg_InterfaceModifiersopt) { this->lpg_InterfaceModifiersopt = lpg_InterfaceModifiersopt; }
        IAst* getAT() { return lpg_AT; };
        void setAT(IAst* lpg_AT) { this->lpg_AT = lpg_AT; }
        IAst* getinterface() { return lpg_interface; };
        void setinterface(IAst* lpg_interface) { this->lpg_interface = lpg_interface; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getAnnotationTypeBody() { return lpg_AnnotationTypeBody; };
        void setAnnotationTypeBody(IAst* lpg_AnnotationTypeBody) { this->lpg_AnnotationTypeBody = lpg_AnnotationTypeBody; }

        AnnotationTypeDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_InterfaceModifiersopt,
            IAst* lpg_AT,
            IAst* lpg_interface,
            IAst* lpg_identifier,
            IAst* lpg_AnnotationTypeBody) :Ast(leftIToken, rightIToken) {
            this->lpg_InterfaceModifiersopt = lpg_InterfaceModifiersopt;
            if (lpg_InterfaceModifiersopt != nullptr) ((Ast*)lpg_InterfaceModifiersopt)->setParent(this);
            this->lpg_AT = lpg_AT;
            ((Ast*)lpg_AT)->setParent(this);
            this->lpg_interface = lpg_interface;
            ((Ast*)lpg_interface)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_AnnotationTypeBody = lpg_AnnotationTypeBody;
            ((Ast*)lpg_AnnotationTypeBody)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_InterfaceModifiersopt);
            list.push_back(lpg_AT);
            list.push_back(lpg_interface);
            list.push_back(lpg_identifier);
            list.push_back(lpg_AnnotationTypeBody);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_InterfaceModifiersopt != nullptr) lpg_InterfaceModifiersopt->accept(v);
                lpg_AT->accept(v);
                lpg_interface->accept(v);
                lpg_identifier->accept(v);
                lpg_AnnotationTypeBody->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 218:  AnnotationTypeBody ::= { AnnotationTypeElementDeclarationsopt }
     *</b>
     */
    struct AnnotationTypeBody :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_AnnotationTypeElementDeclarationsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getAnnotationTypeElementDeclarationsopt</b> may be <b>nullptr</b>
         */
        IAst* getAnnotationTypeElementDeclarationsopt() { return lpg_AnnotationTypeElementDeclarationsopt; };
        void setAnnotationTypeElementDeclarationsopt(IAst* lpg_AnnotationTypeElementDeclarationsopt) { this->lpg_AnnotationTypeElementDeclarationsopt = lpg_AnnotationTypeElementDeclarationsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        AnnotationTypeBody(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_AnnotationTypeElementDeclarationsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_AnnotationTypeElementDeclarationsopt = lpg_AnnotationTypeElementDeclarationsopt;
            if (lpg_AnnotationTypeElementDeclarationsopt != nullptr) ((Ast*)lpg_AnnotationTypeElementDeclarationsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_AnnotationTypeElementDeclarationsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_AnnotationTypeElementDeclarationsopt != nullptr) lpg_AnnotationTypeElementDeclarationsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 219:  AnnotationTypeElementDeclarations ::= AnnotationTypeElementDeclaration
     *</em>
     *<p>
     *<b>
     *<li>Rule 220:  AnnotationTypeElementDeclarations ::= AnnotationTypeElementDeclarations AnnotationTypeElementDeclaration
     *</b>
     */
    struct AnnotationTypeElementDeclarations :public Ast
    {
        IAst* lpg_AnnotationTypeElementDeclarations;
        IAst* lpg_AnnotationTypeElementDeclaration;

        IAst* getAnnotationTypeElementDeclarations() { return lpg_AnnotationTypeElementDeclarations; };
        void setAnnotationTypeElementDeclarations(IAst* lpg_AnnotationTypeElementDeclarations) { this->lpg_AnnotationTypeElementDeclarations = lpg_AnnotationTypeElementDeclarations; }
        IAst* getAnnotationTypeElementDeclaration() { return lpg_AnnotationTypeElementDeclaration; };
        void setAnnotationTypeElementDeclaration(IAst* lpg_AnnotationTypeElementDeclaration) { this->lpg_AnnotationTypeElementDeclaration = lpg_AnnotationTypeElementDeclaration; }

        AnnotationTypeElementDeclarations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AnnotationTypeElementDeclarations,
            IAst* lpg_AnnotationTypeElementDeclaration) :Ast(leftIToken, rightIToken) {
            this->lpg_AnnotationTypeElementDeclarations = lpg_AnnotationTypeElementDeclarations;
            ((Ast*)lpg_AnnotationTypeElementDeclarations)->setParent(this);
            this->lpg_AnnotationTypeElementDeclaration = lpg_AnnotationTypeElementDeclaration;
            ((Ast*)lpg_AnnotationTypeElementDeclaration)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AnnotationTypeElementDeclarations);
            list.push_back(lpg_AnnotationTypeElementDeclaration);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AnnotationTypeElementDeclarations->accept(v);
                lpg_AnnotationTypeElementDeclaration->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 228:  DefaultValue ::= default ElementValue
     *</b>
     */
    struct DefaultValue :public Ast
    {
        IAst* lpg_default;
        IAst* lpg_ElementValue;

        IAst* getdefault() { return lpg_default; };
        void setdefault(IAst* lpg_default) { this->lpg_default = lpg_default; }
        IAst* getElementValue() { return lpg_ElementValue; };
        void setElementValue(IAst* lpg_ElementValue) { this->lpg_ElementValue = lpg_ElementValue; }

        DefaultValue(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_default,
            IAst* lpg_ElementValue) :Ast(leftIToken, rightIToken) {
            this->lpg_default = lpg_default;
            ((Ast*)lpg_default)->setParent(this);
            this->lpg_ElementValue = lpg_ElementValue;
            ((Ast*)lpg_ElementValue)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_default);
            list.push_back(lpg_ElementValue);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_default->accept(v);
                lpg_ElementValue->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 229:  Annotations ::= Annotation
     *</em>
     *<p>
     *<b>
     *<li>Rule 230:  Annotations ::= Annotations Annotation
     *</b>
     */
    struct Annotations :public Ast
    {
        IAst* lpg_Annotations;
        IAst* lpg_Annotation;

        IAst* getAnnotations() { return lpg_Annotations; };
        void setAnnotations(IAst* lpg_Annotations) { this->lpg_Annotations = lpg_Annotations; }
        IAst* getAnnotation() { return lpg_Annotation; };
        void setAnnotation(IAst* lpg_Annotation) { this->lpg_Annotation = lpg_Annotation; }

        Annotations(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Annotations,
            IAst* lpg_Annotation) :Ast(leftIToken, rightIToken) {
            this->lpg_Annotations = lpg_Annotations;
            ((Ast*)lpg_Annotations)->setParent(this);
            this->lpg_Annotation = lpg_Annotation;
            ((Ast*)lpg_Annotation)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Annotations);
            list.push_back(lpg_Annotation);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Annotations->accept(v);
                lpg_Annotation->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 234:  NormalAnnotation ::= @ TypeName ( ElementValuePairsopt )
     *</b>
     */
    struct NormalAnnotation :public Ast
    {
        IAst* lpg_AT;
        IAst* lpg_TypeName;
        IAst* lpg_LPAREN;
        IAst* lpg_ElementValuePairsopt;
        IAst* lpg_RPAREN;

        IAst* getAT() { return lpg_AT; };
        void setAT(IAst* lpg_AT) { this->lpg_AT = lpg_AT; }
        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getElementValuePairsopt</b> may be <b>nullptr</b>
         */
        IAst* getElementValuePairsopt() { return lpg_ElementValuePairsopt; };
        void setElementValuePairsopt(IAst* lpg_ElementValuePairsopt) { this->lpg_ElementValuePairsopt = lpg_ElementValuePairsopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        NormalAnnotation(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AT,
            IAst* lpg_TypeName,
            IAst* lpg_LPAREN,
            IAst* lpg_ElementValuePairsopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_AT = lpg_AT;
            ((Ast*)lpg_AT)->setParent(this);
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ElementValuePairsopt = lpg_ElementValuePairsopt;
            if (lpg_ElementValuePairsopt != nullptr) ((Ast*)lpg_ElementValuePairsopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AT);
            list.push_back(lpg_TypeName);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ElementValuePairsopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AT->accept(v);
                lpg_TypeName->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ElementValuePairsopt != nullptr) lpg_ElementValuePairsopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 235:  ElementValuePairs ::= ElementValuePair
     *</em>
     *<p>
     *<b>
     *<li>Rule 236:  ElementValuePairs ::= ElementValuePairs , ElementValuePair
     *</b>
     */
    struct ElementValuePairs :public Ast
    {
        IAst* lpg_ElementValuePairs;
        IAst* lpg_COMMA;
        IAst* lpg_ElementValuePair;

        IAst* getElementValuePairs() { return lpg_ElementValuePairs; };
        void setElementValuePairs(IAst* lpg_ElementValuePairs) { this->lpg_ElementValuePairs = lpg_ElementValuePairs; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getElementValuePair() { return lpg_ElementValuePair; };
        void setElementValuePair(IAst* lpg_ElementValuePair) { this->lpg_ElementValuePair = lpg_ElementValuePair; }

        ElementValuePairs(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ElementValuePairs,
            IAst* lpg_COMMA,
            IAst* lpg_ElementValuePair) :Ast(leftIToken, rightIToken) {
            this->lpg_ElementValuePairs = lpg_ElementValuePairs;
            ((Ast*)lpg_ElementValuePairs)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_ElementValuePair = lpg_ElementValuePair;
            ((Ast*)lpg_ElementValuePair)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ElementValuePairs);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_ElementValuePair);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ElementValuePairs->accept(v);
                lpg_COMMA->accept(v);
                lpg_ElementValuePair->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 237:  ElementValuePair ::= SimpleName = ElementValue
     *</b>
     */
    struct ElementValuePair :public Ast
    {
        IAst* lpg_SimpleName;
        IAst* lpg_EQUAL;
        IAst* lpg_ElementValue;

        IAst* getSimpleName() { return lpg_SimpleName; };
        void setSimpleName(IAst* lpg_SimpleName) { this->lpg_SimpleName = lpg_SimpleName; }
        IAst* getEQUAL() { return lpg_EQUAL; };
        void setEQUAL(IAst* lpg_EQUAL) { this->lpg_EQUAL = lpg_EQUAL; }
        IAst* getElementValue() { return lpg_ElementValue; };
        void setElementValue(IAst* lpg_ElementValue) { this->lpg_ElementValue = lpg_ElementValue; }

        ElementValuePair(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_SimpleName,
            IAst* lpg_EQUAL,
            IAst* lpg_ElementValue) :Ast(leftIToken, rightIToken) {
            this->lpg_SimpleName = lpg_SimpleName;
            ((Ast*)lpg_SimpleName)->setParent(this);
            this->lpg_EQUAL = lpg_EQUAL;
            ((Ast*)lpg_EQUAL)->setParent(this);
            this->lpg_ElementValue = lpg_ElementValue;
            ((Ast*)lpg_ElementValue)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_SimpleName);
            list.push_back(lpg_EQUAL);
            list.push_back(lpg_ElementValue);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_SimpleName->accept(v);
                lpg_EQUAL->accept(v);
                lpg_ElementValue->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 242:  ElementValueArrayInitializer ::= { ElementValuesopt ,opt }
     *</b>
     */
    struct ElementValueArrayInitializer :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_ElementValuesopt;
        IAst* lpg_Commaopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getElementValuesopt</b> may be <b>nullptr</b>
         */
        IAst* getElementValuesopt() { return lpg_ElementValuesopt; };
        void setElementValuesopt(IAst* lpg_ElementValuesopt) { this->lpg_ElementValuesopt = lpg_ElementValuesopt; }
        /**
         * The value returned by <b>getCommaopt</b> may be <b>nullptr</b>
         */
        IAst* getCommaopt() { return lpg_Commaopt; };
        void setCommaopt(IAst* lpg_Commaopt) { this->lpg_Commaopt = lpg_Commaopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        ElementValueArrayInitializer(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_ElementValuesopt,
            IAst* lpg_Commaopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_ElementValuesopt = lpg_ElementValuesopt;
            if (lpg_ElementValuesopt != nullptr) ((Ast*)lpg_ElementValuesopt)->setParent(this);
            this->lpg_Commaopt = lpg_Commaopt;
            if (lpg_Commaopt != nullptr) ((Ast*)lpg_Commaopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_ElementValuesopt);
            list.push_back(lpg_Commaopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_ElementValuesopt != nullptr) lpg_ElementValuesopt->accept(v);
                if (lpg_Commaopt != nullptr) lpg_Commaopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 243:  ElementValues ::= ElementValue
     *</em>
     *<p>
     *<b>
     *<li>Rule 244:  ElementValues ::= ElementValues , ElementValue
     *</b>
     */
    struct ElementValues :public Ast
    {
        IAst* lpg_ElementValues;
        IAst* lpg_COMMA;
        IAst* lpg_ElementValue;

        IAst* getElementValues() { return lpg_ElementValues; };
        void setElementValues(IAst* lpg_ElementValues) { this->lpg_ElementValues = lpg_ElementValues; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getElementValue() { return lpg_ElementValue; };
        void setElementValue(IAst* lpg_ElementValue) { this->lpg_ElementValue = lpg_ElementValue; }

        ElementValues(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ElementValues,
            IAst* lpg_COMMA,
            IAst* lpg_ElementValue) :Ast(leftIToken, rightIToken) {
            this->lpg_ElementValues = lpg_ElementValues;
            ((Ast*)lpg_ElementValues)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_ElementValue = lpg_ElementValue;
            ((Ast*)lpg_ElementValue)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ElementValues);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_ElementValue);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ElementValues->accept(v);
                lpg_COMMA->accept(v);
                lpg_ElementValue->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 245:  MarkerAnnotation ::= @ TypeName
     *</b>
     */
    struct MarkerAnnotation :public Ast
    {
        IAst* lpg_AT;
        IAst* lpg_TypeName;

        IAst* getAT() { return lpg_AT; };
        void setAT(IAst* lpg_AT) { this->lpg_AT = lpg_AT; }
        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }

        MarkerAnnotation(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AT,
            IAst* lpg_TypeName) :Ast(leftIToken, rightIToken) {
            this->lpg_AT = lpg_AT;
            ((Ast*)lpg_AT)->setParent(this);
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AT);
            list.push_back(lpg_TypeName);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AT->accept(v);
                lpg_TypeName->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 246:  SingleElementAnnotation ::= @ TypeName ( ElementValue )
     *</b>
     */
    struct SingleElementAnnotation :public Ast
    {
        IAst* lpg_AT;
        IAst* lpg_TypeName;
        IAst* lpg_LPAREN;
        IAst* lpg_ElementValue;
        IAst* lpg_RPAREN;

        IAst* getAT() { return lpg_AT; };
        void setAT(IAst* lpg_AT) { this->lpg_AT = lpg_AT; }
        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getElementValue() { return lpg_ElementValue; };
        void setElementValue(IAst* lpg_ElementValue) { this->lpg_ElementValue = lpg_ElementValue; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        SingleElementAnnotation(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AT,
            IAst* lpg_TypeName,
            IAst* lpg_LPAREN,
            IAst* lpg_ElementValue,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_AT = lpg_AT;
            ((Ast*)lpg_AT)->setParent(this);
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ElementValue = lpg_ElementValue;
            ((Ast*)lpg_ElementValue)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AT);
            list.push_back(lpg_TypeName);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ElementValue);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AT->accept(v);
                lpg_TypeName->accept(v);
                lpg_LPAREN->accept(v);
                lpg_ElementValue->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 247:  ArrayInitializer ::= { VariableInitializersopt ,opt }
     *</b>
     */
    struct ArrayInitializer :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_VariableInitializersopt;
        IAst* lpg_Commaopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getVariableInitializersopt</b> may be <b>nullptr</b>
         */
        IAst* getVariableInitializersopt() { return lpg_VariableInitializersopt; };
        void setVariableInitializersopt(IAst* lpg_VariableInitializersopt) { this->lpg_VariableInitializersopt = lpg_VariableInitializersopt; }
        /**
         * The value returned by <b>getCommaopt</b> may be <b>nullptr</b>
         */
        IAst* getCommaopt() { return lpg_Commaopt; };
        void setCommaopt(IAst* lpg_Commaopt) { this->lpg_Commaopt = lpg_Commaopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        ArrayInitializer(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_VariableInitializersopt,
            IAst* lpg_Commaopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_VariableInitializersopt = lpg_VariableInitializersopt;
            if (lpg_VariableInitializersopt != nullptr) ((Ast*)lpg_VariableInitializersopt)->setParent(this);
            this->lpg_Commaopt = lpg_Commaopt;
            if (lpg_Commaopt != nullptr) ((Ast*)lpg_Commaopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_VariableInitializersopt);
            list.push_back(lpg_Commaopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_VariableInitializersopt != nullptr) lpg_VariableInitializersopt->accept(v);
                if (lpg_Commaopt != nullptr) lpg_Commaopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 248:  VariableInitializers ::= VariableInitializer
     *</em>
     *<p>
     *<b>
     *<li>Rule 249:  VariableInitializers ::= VariableInitializers , VariableInitializer
     *</b>
     */
    struct VariableInitializers :public Ast
    {
        IAst* lpg_VariableInitializers;
        IAst* lpg_COMMA;
        IAst* lpg_VariableInitializer;

        IAst* getVariableInitializers() { return lpg_VariableInitializers; };
        void setVariableInitializers(IAst* lpg_VariableInitializers) { this->lpg_VariableInitializers = lpg_VariableInitializers; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getVariableInitializer() { return lpg_VariableInitializer; };
        void setVariableInitializer(IAst* lpg_VariableInitializer) { this->lpg_VariableInitializer = lpg_VariableInitializer; }

        VariableInitializers(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableInitializers,
            IAst* lpg_COMMA,
            IAst* lpg_VariableInitializer) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableInitializers = lpg_VariableInitializers;
            ((Ast*)lpg_VariableInitializers)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_VariableInitializer = lpg_VariableInitializer;
            ((Ast*)lpg_VariableInitializer)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableInitializers);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_VariableInitializer);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_VariableInitializers->accept(v);
                lpg_COMMA->accept(v);
                lpg_VariableInitializer->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 250:  Block ::= { BlockStatementsopt }
     *</b>
     */
    struct Block :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_BlockStatementsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getBlockStatementsopt</b> may be <b>nullptr</b>
         */
        IAst* getBlockStatementsopt() { return lpg_BlockStatementsopt; };
        void setBlockStatementsopt(IAst* lpg_BlockStatementsopt) { this->lpg_BlockStatementsopt = lpg_BlockStatementsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        Block(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_BlockStatementsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_BlockStatementsopt = lpg_BlockStatementsopt;
            if (lpg_BlockStatementsopt != nullptr) ((Ast*)lpg_BlockStatementsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_BlockStatementsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_BlockStatementsopt != nullptr) lpg_BlockStatementsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 251:  BlockStatements ::= BlockStatement
     *</em>
     *<p>
     *<b>
     *<li>Rule 252:  BlockStatements ::= BlockStatements BlockStatement
     *</b>
     */
    struct BlockStatements :public Ast
    {
        IAst* lpg_BlockStatements;
        IAst* lpg_BlockStatement;

        IAst* getBlockStatements() { return lpg_BlockStatements; };
        void setBlockStatements(IAst* lpg_BlockStatements) { this->lpg_BlockStatements = lpg_BlockStatements; }
        IAst* getBlockStatement() { return lpg_BlockStatement; };
        void setBlockStatement(IAst* lpg_BlockStatement) { this->lpg_BlockStatement = lpg_BlockStatement; }

        BlockStatements(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_BlockStatements,
            IAst* lpg_BlockStatement) :Ast(leftIToken, rightIToken) {
            this->lpg_BlockStatements = lpg_BlockStatements;
            ((Ast*)lpg_BlockStatements)->setParent(this);
            this->lpg_BlockStatement = lpg_BlockStatement;
            ((Ast*)lpg_BlockStatement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_BlockStatements);
            list.push_back(lpg_BlockStatement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_BlockStatements->accept(v);
                lpg_BlockStatement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 256:  LocalVariableDeclarationStatement ::= LocalVariableDeclaration ;
     *</b>
     */
    struct LocalVariableDeclarationStatement :public Ast
    {
        IAst* lpg_LocalVariableDeclaration;
        IAst* lpg_SEMICOLON;

        IAst* getLocalVariableDeclaration() { return lpg_LocalVariableDeclaration; };
        void setLocalVariableDeclaration(IAst* lpg_LocalVariableDeclaration) { this->lpg_LocalVariableDeclaration = lpg_LocalVariableDeclaration; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        LocalVariableDeclarationStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LocalVariableDeclaration,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_LocalVariableDeclaration = lpg_LocalVariableDeclaration;
            ((Ast*)lpg_LocalVariableDeclaration)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LocalVariableDeclaration);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LocalVariableDeclaration->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 257:  LocalVariableDeclaration ::= VariableModifiersopt Type VariableDeclarators
     *</b>
     */
    struct LocalVariableDeclaration :public Ast
    {
        IAst* lpg_VariableModifiersopt;
        IAst* lpg_Type;
        IAst* lpg_VariableDeclarators;

        /**
         * The value returned by <b>getVariableModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getVariableModifiersopt() { return lpg_VariableModifiersopt; };
        void setVariableModifiersopt(IAst* lpg_VariableModifiersopt) { this->lpg_VariableModifiersopt = lpg_VariableModifiersopt; }
        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getVariableDeclarators() { return lpg_VariableDeclarators; };
        void setVariableDeclarators(IAst* lpg_VariableDeclarators) { this->lpg_VariableDeclarators = lpg_VariableDeclarators; }

        LocalVariableDeclaration(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_VariableModifiersopt,
            IAst* lpg_Type,
            IAst* lpg_VariableDeclarators) :Ast(leftIToken, rightIToken) {
            this->lpg_VariableModifiersopt = lpg_VariableModifiersopt;
            if (lpg_VariableModifiersopt != nullptr) ((Ast*)lpg_VariableModifiersopt)->setParent(this);
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_VariableDeclarators = lpg_VariableDeclarators;
            ((Ast*)lpg_VariableDeclarators)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_VariableModifiersopt);
            list.push_back(lpg_Type);
            list.push_back(lpg_VariableDeclarators);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_VariableModifiersopt != nullptr) lpg_VariableModifiersopt->accept(v);
                lpg_Type->accept(v);
                lpg_VariableDeclarators->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 281:  IfThenStatement ::= if ( Expression ) Statement
     *</b>
     */
    struct IfThenStatement :public Ast
    {
        IAst* lpg_if;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_Statement;

        IAst* getif() { return lpg_if; };
        void setif(IAst* lpg_if) { this->lpg_if = lpg_if; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }

        IfThenStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_if,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_Statement) :Ast(leftIToken, rightIToken) {
            this->lpg_if = lpg_if;
            ((Ast*)lpg_if)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_if);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_Statement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_if->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_Statement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 282:  IfThenElseStatement ::= if ( Expression ) StatementNoShortIf else Statement
     *</b>
     */
    struct IfThenElseStatement :public Ast
    {
        IAst* lpg_if;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_StatementNoShortIf;
        IAst* lpg_else;
        IAst* lpg_Statement;

        IAst* getif() { return lpg_if; };
        void setif(IAst* lpg_if) { this->lpg_if = lpg_if; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatementNoShortIf() { return lpg_StatementNoShortIf; };
        void setStatementNoShortIf(IAst* lpg_StatementNoShortIf) { this->lpg_StatementNoShortIf = lpg_StatementNoShortIf; }
        IAst* getelse() { return lpg_else; };
        void setelse(IAst* lpg_else) { this->lpg_else = lpg_else; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }

        IfThenElseStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_if,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_StatementNoShortIf,
            IAst* lpg_else,
            IAst* lpg_Statement) :Ast(leftIToken, rightIToken) {
            this->lpg_if = lpg_if;
            ((Ast*)lpg_if)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_StatementNoShortIf = lpg_StatementNoShortIf;
            ((Ast*)lpg_StatementNoShortIf)->setParent(this);
            this->lpg_else = lpg_else;
            ((Ast*)lpg_else)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_if);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_StatementNoShortIf);
            list.push_back(lpg_else);
            list.push_back(lpg_Statement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_if->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_StatementNoShortIf->accept(v);
                lpg_else->accept(v);
                lpg_Statement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 283:  IfThenElseStatementNoShortIf ::= if ( Expression ) StatementNoShortIf else StatementNoShortIf
     *</b>
     */
    struct IfThenElseStatementNoShortIf :public Ast
    {
        IAst* lpg_if;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_StatementNoShortIf;
        IAst* lpg_else;
        IAst* lpg_StatementNoShortIf7;

        IAst* getif() { return lpg_if; };
        void setif(IAst* lpg_if) { this->lpg_if = lpg_if; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatementNoShortIf() { return lpg_StatementNoShortIf; };
        void setStatementNoShortIf(IAst* lpg_StatementNoShortIf) { this->lpg_StatementNoShortIf = lpg_StatementNoShortIf; }
        IAst* getelse() { return lpg_else; };
        void setelse(IAst* lpg_else) { this->lpg_else = lpg_else; }
        IAst* getStatementNoShortIf7() { return lpg_StatementNoShortIf7; };
        void setStatementNoShortIf7(IAst* lpg_StatementNoShortIf7) { this->lpg_StatementNoShortIf7 = lpg_StatementNoShortIf7; }

        IfThenElseStatementNoShortIf(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_if,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_StatementNoShortIf,
            IAst* lpg_else,
            IAst* lpg_StatementNoShortIf7) :Ast(leftIToken, rightIToken) {
            this->lpg_if = lpg_if;
            ((Ast*)lpg_if)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_StatementNoShortIf = lpg_StatementNoShortIf;
            ((Ast*)lpg_StatementNoShortIf)->setParent(this);
            this->lpg_else = lpg_else;
            ((Ast*)lpg_else)->setParent(this);
            this->lpg_StatementNoShortIf7 = lpg_StatementNoShortIf7;
            ((Ast*)lpg_StatementNoShortIf7)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_if);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_StatementNoShortIf);
            list.push_back(lpg_else);
            list.push_back(lpg_StatementNoShortIf7);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_if->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_StatementNoShortIf->accept(v);
                lpg_else->accept(v);
                lpg_StatementNoShortIf7->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 284:  EmptyStatement ::= ;
     *</b>
     */
    struct EmptyStatement :public AstToken
    {
        IToken* getSEMICOLON() { return leftIToken; }

        EmptyStatement(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 285:  LabeledStatement ::= identifier : Statement
     *</b>
     */
    struct LabeledStatement :public Ast
    {
        IAst* lpg_identifier;
        IAst* lpg_COLON;
        IAst* lpg_Statement;

        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }

        LabeledStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_identifier,
            IAst* lpg_COLON,
            IAst* lpg_Statement) :Ast(leftIToken, rightIToken) {
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_identifier);
            list.push_back(lpg_COLON);
            list.push_back(lpg_Statement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_identifier->accept(v);
                lpg_COLON->accept(v);
                lpg_Statement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 286:  LabeledStatementNoShortIf ::= identifier : StatementNoShortIf
     *</b>
     */
    struct LabeledStatementNoShortIf :public Ast
    {
        IAst* lpg_identifier;
        IAst* lpg_COLON;
        IAst* lpg_StatementNoShortIf;

        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }
        IAst* getStatementNoShortIf() { return lpg_StatementNoShortIf; };
        void setStatementNoShortIf(IAst* lpg_StatementNoShortIf) { this->lpg_StatementNoShortIf = lpg_StatementNoShortIf; }

        LabeledStatementNoShortIf(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_identifier,
            IAst* lpg_COLON,
            IAst* lpg_StatementNoShortIf) :Ast(leftIToken, rightIToken) {
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            this->lpg_StatementNoShortIf = lpg_StatementNoShortIf;
            ((Ast*)lpg_StatementNoShortIf)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_identifier);
            list.push_back(lpg_COLON);
            list.push_back(lpg_StatementNoShortIf);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_identifier->accept(v);
                lpg_COLON->accept(v);
                lpg_StatementNoShortIf->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 287:  ExpressionStatement ::= StatementExpression ;
     *</b>
     */
    struct ExpressionStatement :public Ast
    {
        IAst* lpg_StatementExpression;
        IAst* lpg_SEMICOLON;

        IAst* getStatementExpression() { return lpg_StatementExpression; };
        void setStatementExpression(IAst* lpg_StatementExpression) { this->lpg_StatementExpression = lpg_StatementExpression; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ExpressionStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_StatementExpression,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_StatementExpression = lpg_StatementExpression;
            ((Ast*)lpg_StatementExpression)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_StatementExpression);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_StatementExpression->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 297:  SwitchStatement ::= switch ( Expression ) SwitchBlock
     *</b>
     */
    struct SwitchStatement :public Ast
    {
        IAst* lpg_switch;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_SwitchBlock;

        IAst* getswitch() { return lpg_switch; };
        void setswitch(IAst* lpg_switch) { this->lpg_switch = lpg_switch; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getSwitchBlock() { return lpg_SwitchBlock; };
        void setSwitchBlock(IAst* lpg_SwitchBlock) { this->lpg_SwitchBlock = lpg_SwitchBlock; }

        SwitchStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_switch,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_SwitchBlock) :Ast(leftIToken, rightIToken) {
            this->lpg_switch = lpg_switch;
            ((Ast*)lpg_switch)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_SwitchBlock = lpg_SwitchBlock;
            ((Ast*)lpg_SwitchBlock)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_switch);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_SwitchBlock);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_switch->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_SwitchBlock->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 298:  SwitchBlock ::= { SwitchBlockStatementGroupsopt SwitchLabelsopt }
     *</b>
     */
    struct SwitchBlock :public Ast
    {
        IAst* lpg_LBRACE;
        IAst* lpg_SwitchBlockStatementGroupsopt;
        IAst* lpg_SwitchLabelsopt;
        IAst* lpg_RBRACE;

        IAst* getLBRACE() { return lpg_LBRACE; };
        void setLBRACE(IAst* lpg_LBRACE) { this->lpg_LBRACE = lpg_LBRACE; }
        /**
         * The value returned by <b>getSwitchBlockStatementGroupsopt</b> may be <b>nullptr</b>
         */
        IAst* getSwitchBlockStatementGroupsopt() { return lpg_SwitchBlockStatementGroupsopt; };
        void setSwitchBlockStatementGroupsopt(IAst* lpg_SwitchBlockStatementGroupsopt) { this->lpg_SwitchBlockStatementGroupsopt = lpg_SwitchBlockStatementGroupsopt; }
        /**
         * The value returned by <b>getSwitchLabelsopt</b> may be <b>nullptr</b>
         */
        IAst* getSwitchLabelsopt() { return lpg_SwitchLabelsopt; };
        void setSwitchLabelsopt(IAst* lpg_SwitchLabelsopt) { this->lpg_SwitchLabelsopt = lpg_SwitchLabelsopt; }
        IAst* getRBRACE() { return lpg_RBRACE; };
        void setRBRACE(IAst* lpg_RBRACE) { this->lpg_RBRACE = lpg_RBRACE; }

        SwitchBlock(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACE,
            IAst* lpg_SwitchBlockStatementGroupsopt,
            IAst* lpg_SwitchLabelsopt,
            IAst* lpg_RBRACE) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACE = lpg_LBRACE;
            ((Ast*)lpg_LBRACE)->setParent(this);
            this->lpg_SwitchBlockStatementGroupsopt = lpg_SwitchBlockStatementGroupsopt;
            if (lpg_SwitchBlockStatementGroupsopt != nullptr) ((Ast*)lpg_SwitchBlockStatementGroupsopt)->setParent(this);
            this->lpg_SwitchLabelsopt = lpg_SwitchLabelsopt;
            if (lpg_SwitchLabelsopt != nullptr) ((Ast*)lpg_SwitchLabelsopt)->setParent(this);
            this->lpg_RBRACE = lpg_RBRACE;
            ((Ast*)lpg_RBRACE)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACE);
            list.push_back(lpg_SwitchBlockStatementGroupsopt);
            list.push_back(lpg_SwitchLabelsopt);
            list.push_back(lpg_RBRACE);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACE->accept(v);
                if (lpg_SwitchBlockStatementGroupsopt != nullptr) lpg_SwitchBlockStatementGroupsopt->accept(v);
                if (lpg_SwitchLabelsopt != nullptr) lpg_SwitchLabelsopt->accept(v);
                lpg_RBRACE->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 299:  SwitchBlockStatementGroups ::= SwitchBlockStatementGroup
     *</em>
     *<p>
     *<b>
     *<li>Rule 300:  SwitchBlockStatementGroups ::= SwitchBlockStatementGroups SwitchBlockStatementGroup
     *</b>
     */
    struct SwitchBlockStatementGroups :public Ast
    {
        IAst* lpg_SwitchBlockStatementGroups;
        IAst* lpg_SwitchBlockStatementGroup;

        IAst* getSwitchBlockStatementGroups() { return lpg_SwitchBlockStatementGroups; };
        void setSwitchBlockStatementGroups(IAst* lpg_SwitchBlockStatementGroups) { this->lpg_SwitchBlockStatementGroups = lpg_SwitchBlockStatementGroups; }
        IAst* getSwitchBlockStatementGroup() { return lpg_SwitchBlockStatementGroup; };
        void setSwitchBlockStatementGroup(IAst* lpg_SwitchBlockStatementGroup) { this->lpg_SwitchBlockStatementGroup = lpg_SwitchBlockStatementGroup; }

        SwitchBlockStatementGroups(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_SwitchBlockStatementGroups,
            IAst* lpg_SwitchBlockStatementGroup) :Ast(leftIToken, rightIToken) {
            this->lpg_SwitchBlockStatementGroups = lpg_SwitchBlockStatementGroups;
            ((Ast*)lpg_SwitchBlockStatementGroups)->setParent(this);
            this->lpg_SwitchBlockStatementGroup = lpg_SwitchBlockStatementGroup;
            ((Ast*)lpg_SwitchBlockStatementGroup)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_SwitchBlockStatementGroups);
            list.push_back(lpg_SwitchBlockStatementGroup);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_SwitchBlockStatementGroups->accept(v);
                lpg_SwitchBlockStatementGroup->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 301:  SwitchBlockStatementGroup ::= SwitchLabels BlockStatements
     *</b>
     */
    struct SwitchBlockStatementGroup :public Ast
    {
        IAst* lpg_SwitchLabels;
        IAst* lpg_BlockStatements;

        IAst* getSwitchLabels() { return lpg_SwitchLabels; };
        void setSwitchLabels(IAst* lpg_SwitchLabels) { this->lpg_SwitchLabels = lpg_SwitchLabels; }
        IAst* getBlockStatements() { return lpg_BlockStatements; };
        void setBlockStatements(IAst* lpg_BlockStatements) { this->lpg_BlockStatements = lpg_BlockStatements; }

        SwitchBlockStatementGroup(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_SwitchLabels,
            IAst* lpg_BlockStatements) :Ast(leftIToken, rightIToken) {
            this->lpg_SwitchLabels = lpg_SwitchLabels;
            ((Ast*)lpg_SwitchLabels)->setParent(this);
            this->lpg_BlockStatements = lpg_BlockStatements;
            ((Ast*)lpg_BlockStatements)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_SwitchLabels);
            list.push_back(lpg_BlockStatements);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_SwitchLabels->accept(v);
                lpg_BlockStatements->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 302:  SwitchLabels ::= SwitchLabel
     *</em>
     *<p>
     *<b>
     *<li>Rule 303:  SwitchLabels ::= SwitchLabels SwitchLabel
     *</b>
     */
    struct SwitchLabels :public Ast
    {
        IAst* lpg_SwitchLabels;
        IAst* lpg_SwitchLabel;

        IAst* getSwitchLabels() { return lpg_SwitchLabels; };
        void setSwitchLabels(IAst* lpg_SwitchLabels) { this->lpg_SwitchLabels = lpg_SwitchLabels; }
        IAst* getSwitchLabel() { return lpg_SwitchLabel; };
        void setSwitchLabel(IAst* lpg_SwitchLabel) { this->lpg_SwitchLabel = lpg_SwitchLabel; }

        SwitchLabels(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_SwitchLabels,
            IAst* lpg_SwitchLabel) :Ast(leftIToken, rightIToken) {
            this->lpg_SwitchLabels = lpg_SwitchLabels;
            ((Ast*)lpg_SwitchLabels)->setParent(this);
            this->lpg_SwitchLabel = lpg_SwitchLabel;
            ((Ast*)lpg_SwitchLabel)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_SwitchLabels);
            list.push_back(lpg_SwitchLabel);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_SwitchLabels->accept(v);
                lpg_SwitchLabel->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 308:  WhileStatement ::= while ( Expression ) Statement
     *</b>
     */
    struct WhileStatement :public Ast
    {
        IAst* lpg_while;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_Statement;

        IAst* getwhile() { return lpg_while; };
        void setwhile(IAst* lpg_while) { this->lpg_while = lpg_while; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }

        WhileStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_while,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_Statement) :Ast(leftIToken, rightIToken) {
            this->lpg_while = lpg_while;
            ((Ast*)lpg_while)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_while);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_Statement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_while->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_Statement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 309:  WhileStatementNoShortIf ::= while ( Expression ) StatementNoShortIf
     *</b>
     */
    struct WhileStatementNoShortIf :public Ast
    {
        IAst* lpg_while;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_StatementNoShortIf;

        IAst* getwhile() { return lpg_while; };
        void setwhile(IAst* lpg_while) { this->lpg_while = lpg_while; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatementNoShortIf() { return lpg_StatementNoShortIf; };
        void setStatementNoShortIf(IAst* lpg_StatementNoShortIf) { this->lpg_StatementNoShortIf = lpg_StatementNoShortIf; }

        WhileStatementNoShortIf(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_while,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_StatementNoShortIf) :Ast(leftIToken, rightIToken) {
            this->lpg_while = lpg_while;
            ((Ast*)lpg_while)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_StatementNoShortIf = lpg_StatementNoShortIf;
            ((Ast*)lpg_StatementNoShortIf)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_while);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_StatementNoShortIf);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_while->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_StatementNoShortIf->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 310:  DoStatement ::= do Statement while ( Expression ) ;
     *</b>
     */
    struct DoStatement :public Ast
    {
        IAst* lpg_do;
        IAst* lpg_Statement;
        IAst* lpg_while;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_SEMICOLON;

        IAst* getdo() { return lpg_do; };
        void setdo(IAst* lpg_do) { this->lpg_do = lpg_do; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }
        IAst* getwhile() { return lpg_while; };
        void setwhile(IAst* lpg_while) { this->lpg_while = lpg_while; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        DoStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_do,
            IAst* lpg_Statement,
            IAst* lpg_while,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_do = lpg_do;
            ((Ast*)lpg_do)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            this->lpg_while = lpg_while;
            ((Ast*)lpg_while)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_do);
            list.push_back(lpg_Statement);
            list.push_back(lpg_while);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_do->accept(v);
                lpg_Statement->accept(v);
                lpg_while->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 313:  BasicForStatement ::= for ( ForInitopt ; Expressionopt ; ForUpdateopt ) Statement
     *</b>
     */
    struct BasicForStatement :public Ast
    {
        IAst* lpg_for;
        IAst* lpg_LPAREN;
        IAst* lpg_ForInitopt;
        IAst* lpg_SEMICOLON;
        IAst* lpg_Expressionopt;
        IAst* lpg_SEMICOLON6;
        IAst* lpg_ForUpdateopt;
        IAst* lpg_RPAREN;
        IAst* lpg_Statement;

        IAst* getfor() { return lpg_for; };
        void setfor(IAst* lpg_for) { this->lpg_for = lpg_for; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getForInitopt</b> may be <b>nullptr</b>
         */
        IAst* getForInitopt() { return lpg_ForInitopt; };
        void setForInitopt(IAst* lpg_ForInitopt) { this->lpg_ForInitopt = lpg_ForInitopt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }
        /**
         * The value returned by <b>getExpressionopt</b> may be <b>nullptr</b>
         */
        IAst* getExpressionopt() { return lpg_Expressionopt; };
        void setExpressionopt(IAst* lpg_Expressionopt) { this->lpg_Expressionopt = lpg_Expressionopt; }
        IAst* getSEMICOLON6() { return lpg_SEMICOLON6; };
        void setSEMICOLON6(IAst* lpg_SEMICOLON6) { this->lpg_SEMICOLON6 = lpg_SEMICOLON6; }
        /**
         * The value returned by <b>getForUpdateopt</b> may be <b>nullptr</b>
         */
        IAst* getForUpdateopt() { return lpg_ForUpdateopt; };
        void setForUpdateopt(IAst* lpg_ForUpdateopt) { this->lpg_ForUpdateopt = lpg_ForUpdateopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }

        BasicForStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_for,
            IAst* lpg_LPAREN,
            IAst* lpg_ForInitopt,
            IAst* lpg_SEMICOLON,
            IAst* lpg_Expressionopt,
            IAst* lpg_SEMICOLON6,
            IAst* lpg_ForUpdateopt,
            IAst* lpg_RPAREN,
            IAst* lpg_Statement) :Ast(leftIToken, rightIToken) {
            this->lpg_for = lpg_for;
            ((Ast*)lpg_for)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ForInitopt = lpg_ForInitopt;
            if (lpg_ForInitopt != nullptr) ((Ast*)lpg_ForInitopt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            this->lpg_Expressionopt = lpg_Expressionopt;
            if (lpg_Expressionopt != nullptr) ((Ast*)lpg_Expressionopt)->setParent(this);
            this->lpg_SEMICOLON6 = lpg_SEMICOLON6;
            ((Ast*)lpg_SEMICOLON6)->setParent(this);
            this->lpg_ForUpdateopt = lpg_ForUpdateopt;
            if (lpg_ForUpdateopt != nullptr) ((Ast*)lpg_ForUpdateopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_for);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ForInitopt);
            list.push_back(lpg_SEMICOLON);
            list.push_back(lpg_Expressionopt);
            list.push_back(lpg_SEMICOLON6);
            list.push_back(lpg_ForUpdateopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_Statement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_for->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ForInitopt != nullptr) lpg_ForInitopt->accept(v);
                lpg_SEMICOLON->accept(v);
                if (lpg_Expressionopt != nullptr) lpg_Expressionopt->accept(v);
                lpg_SEMICOLON6->accept(v);
                if (lpg_ForUpdateopt != nullptr) lpg_ForUpdateopt->accept(v);
                lpg_RPAREN->accept(v);
                lpg_Statement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 314:  ForStatementNoShortIf ::= for ( ForInitopt ; Expressionopt ; ForUpdateopt ) StatementNoShortIf
     *</b>
     */
    struct ForStatementNoShortIf :public Ast
    {
        IAst* lpg_for;
        IAst* lpg_LPAREN;
        IAst* lpg_ForInitopt;
        IAst* lpg_SEMICOLON;
        IAst* lpg_Expressionopt;
        IAst* lpg_SEMICOLON6;
        IAst* lpg_ForUpdateopt;
        IAst* lpg_RPAREN;
        IAst* lpg_StatementNoShortIf;

        IAst* getfor() { return lpg_for; };
        void setfor(IAst* lpg_for) { this->lpg_for = lpg_for; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getForInitopt</b> may be <b>nullptr</b>
         */
        IAst* getForInitopt() { return lpg_ForInitopt; };
        void setForInitopt(IAst* lpg_ForInitopt) { this->lpg_ForInitopt = lpg_ForInitopt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }
        /**
         * The value returned by <b>getExpressionopt</b> may be <b>nullptr</b>
         */
        IAst* getExpressionopt() { return lpg_Expressionopt; };
        void setExpressionopt(IAst* lpg_Expressionopt) { this->lpg_Expressionopt = lpg_Expressionopt; }
        IAst* getSEMICOLON6() { return lpg_SEMICOLON6; };
        void setSEMICOLON6(IAst* lpg_SEMICOLON6) { this->lpg_SEMICOLON6 = lpg_SEMICOLON6; }
        /**
         * The value returned by <b>getForUpdateopt</b> may be <b>nullptr</b>
         */
        IAst* getForUpdateopt() { return lpg_ForUpdateopt; };
        void setForUpdateopt(IAst* lpg_ForUpdateopt) { this->lpg_ForUpdateopt = lpg_ForUpdateopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatementNoShortIf() { return lpg_StatementNoShortIf; };
        void setStatementNoShortIf(IAst* lpg_StatementNoShortIf) { this->lpg_StatementNoShortIf = lpg_StatementNoShortIf; }

        ForStatementNoShortIf(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_for,
            IAst* lpg_LPAREN,
            IAst* lpg_ForInitopt,
            IAst* lpg_SEMICOLON,
            IAst* lpg_Expressionopt,
            IAst* lpg_SEMICOLON6,
            IAst* lpg_ForUpdateopt,
            IAst* lpg_RPAREN,
            IAst* lpg_StatementNoShortIf) :Ast(leftIToken, rightIToken) {
            this->lpg_for = lpg_for;
            ((Ast*)lpg_for)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ForInitopt = lpg_ForInitopt;
            if (lpg_ForInitopt != nullptr) ((Ast*)lpg_ForInitopt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            this->lpg_Expressionopt = lpg_Expressionopt;
            if (lpg_Expressionopt != nullptr) ((Ast*)lpg_Expressionopt)->setParent(this);
            this->lpg_SEMICOLON6 = lpg_SEMICOLON6;
            ((Ast*)lpg_SEMICOLON6)->setParent(this);
            this->lpg_ForUpdateopt = lpg_ForUpdateopt;
            if (lpg_ForUpdateopt != nullptr) ((Ast*)lpg_ForUpdateopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_StatementNoShortIf = lpg_StatementNoShortIf;
            ((Ast*)lpg_StatementNoShortIf)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_for);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ForInitopt);
            list.push_back(lpg_SEMICOLON);
            list.push_back(lpg_Expressionopt);
            list.push_back(lpg_SEMICOLON6);
            list.push_back(lpg_ForUpdateopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_StatementNoShortIf);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_for->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ForInitopt != nullptr) lpg_ForInitopt->accept(v);
                lpg_SEMICOLON->accept(v);
                if (lpg_Expressionopt != nullptr) lpg_Expressionopt->accept(v);
                lpg_SEMICOLON6->accept(v);
                if (lpg_ForUpdateopt != nullptr) lpg_ForUpdateopt->accept(v);
                lpg_RPAREN->accept(v);
                lpg_StatementNoShortIf->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 318:  StatementExpressionList ::= StatementExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 319:  StatementExpressionList ::= StatementExpressionList , StatementExpression
     *</b>
     */
    struct StatementExpressionList :public Ast
    {
        IAst* lpg_StatementExpressionList;
        IAst* lpg_COMMA;
        IAst* lpg_StatementExpression;

        IAst* getStatementExpressionList() { return lpg_StatementExpressionList; };
        void setStatementExpressionList(IAst* lpg_StatementExpressionList) { this->lpg_StatementExpressionList = lpg_StatementExpressionList; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getStatementExpression() { return lpg_StatementExpression; };
        void setStatementExpression(IAst* lpg_StatementExpression) { this->lpg_StatementExpression = lpg_StatementExpression; }

        StatementExpressionList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_StatementExpressionList,
            IAst* lpg_COMMA,
            IAst* lpg_StatementExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_StatementExpressionList = lpg_StatementExpressionList;
            ((Ast*)lpg_StatementExpressionList)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_StatementExpression = lpg_StatementExpression;
            ((Ast*)lpg_StatementExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_StatementExpressionList);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_StatementExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_StatementExpressionList->accept(v);
                lpg_COMMA->accept(v);
                lpg_StatementExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 320:  EnhancedForStatement ::= for ( FormalParameter : Expression ) Statement
     *</b>
     */
    struct EnhancedForStatement :public Ast
    {
        IAst* lpg_for;
        IAst* lpg_LPAREN;
        IAst* lpg_FormalParameter;
        IAst* lpg_COLON;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_Statement;

        IAst* getfor() { return lpg_for; };
        void setfor(IAst* lpg_for) { this->lpg_for = lpg_for; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getFormalParameter() { return lpg_FormalParameter; };
        void setFormalParameter(IAst* lpg_FormalParameter) { this->lpg_FormalParameter = lpg_FormalParameter; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getStatement() { return lpg_Statement; };
        void setStatement(IAst* lpg_Statement) { this->lpg_Statement = lpg_Statement; }

        EnhancedForStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_for,
            IAst* lpg_LPAREN,
            IAst* lpg_FormalParameter,
            IAst* lpg_COLON,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_Statement) :Ast(leftIToken, rightIToken) {
            this->lpg_for = lpg_for;
            ((Ast*)lpg_for)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_FormalParameter = lpg_FormalParameter;
            ((Ast*)lpg_FormalParameter)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_Statement = lpg_Statement;
            ((Ast*)lpg_Statement)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_for);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_FormalParameter);
            list.push_back(lpg_COLON);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_Statement);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_for->accept(v);
                lpg_LPAREN->accept(v);
                lpg_FormalParameter->accept(v);
                lpg_COLON->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_Statement->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 321:  BreakStatement ::= break identifieropt ;
     *</b>
     */
    struct BreakStatement :public Ast
    {
        IAst* lpg_break;
        IAst* lpg_identifieropt;
        IAst* lpg_SEMICOLON;

        IAst* getbreak() { return lpg_break; };
        void setbreak(IAst* lpg_break) { this->lpg_break = lpg_break; }
        /**
         * The value returned by <b>getidentifieropt</b> may be <b>nullptr</b>
         */
        IAst* getidentifieropt() { return lpg_identifieropt; };
        void setidentifieropt(IAst* lpg_identifieropt) { this->lpg_identifieropt = lpg_identifieropt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        BreakStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_break,
            IAst* lpg_identifieropt,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_break = lpg_break;
            ((Ast*)lpg_break)->setParent(this);
            this->lpg_identifieropt = lpg_identifieropt;
            if (lpg_identifieropt != nullptr) ((Ast*)lpg_identifieropt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_break);
            list.push_back(lpg_identifieropt);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_break->accept(v);
                if (lpg_identifieropt != nullptr) lpg_identifieropt->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 322:  ContinueStatement ::= continue identifieropt ;
     *</b>
     */
    struct ContinueStatement :public Ast
    {
        IAst* lpg_continue;
        IAst* lpg_identifieropt;
        IAst* lpg_SEMICOLON;

        IAst* getcontinue() { return lpg_continue; };
        void setcontinue(IAst* lpg_continue) { this->lpg_continue = lpg_continue; }
        /**
         * The value returned by <b>getidentifieropt</b> may be <b>nullptr</b>
         */
        IAst* getidentifieropt() { return lpg_identifieropt; };
        void setidentifieropt(IAst* lpg_identifieropt) { this->lpg_identifieropt = lpg_identifieropt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ContinueStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_continue,
            IAst* lpg_identifieropt,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_continue = lpg_continue;
            ((Ast*)lpg_continue)->setParent(this);
            this->lpg_identifieropt = lpg_identifieropt;
            if (lpg_identifieropt != nullptr) ((Ast*)lpg_identifieropt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_continue);
            list.push_back(lpg_identifieropt);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_continue->accept(v);
                if (lpg_identifieropt != nullptr) lpg_identifieropt->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 323:  ReturnStatement ::= return Expressionopt ;
     *</b>
     */
    struct ReturnStatement :public Ast
    {
        IAst* lpg_return;
        IAst* lpg_Expressionopt;
        IAst* lpg_SEMICOLON;

        IAst* getreturn() { return lpg_return; };
        void setreturn(IAst* lpg_return) { this->lpg_return = lpg_return; }
        /**
         * The value returned by <b>getExpressionopt</b> may be <b>nullptr</b>
         */
        IAst* getExpressionopt() { return lpg_Expressionopt; };
        void setExpressionopt(IAst* lpg_Expressionopt) { this->lpg_Expressionopt = lpg_Expressionopt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ReturnStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_return,
            IAst* lpg_Expressionopt,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_return = lpg_return;
            ((Ast*)lpg_return)->setParent(this);
            this->lpg_Expressionopt = lpg_Expressionopt;
            if (lpg_Expressionopt != nullptr) ((Ast*)lpg_Expressionopt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_return);
            list.push_back(lpg_Expressionopt);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_return->accept(v);
                if (lpg_Expressionopt != nullptr) lpg_Expressionopt->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 324:  ThrowStatement ::= throw Expression ;
     *</b>
     */
    struct ThrowStatement :public Ast
    {
        IAst* lpg_throw;
        IAst* lpg_Expression;
        IAst* lpg_SEMICOLON;

        IAst* getthrow() { return lpg_throw; };
        void setthrow(IAst* lpg_throw) { this->lpg_throw = lpg_throw; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ThrowStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_throw,
            IAst* lpg_Expression,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_throw = lpg_throw;
            ((Ast*)lpg_throw)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_throw);
            list.push_back(lpg_Expression);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_throw->accept(v);
                lpg_Expression->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 325:  SynchronizedStatement ::= synchronized ( Expression ) Block
     *</b>
     */
    struct SynchronizedStatement :public Ast
    {
        IAst* lpg_synchronized;
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;
        IAst* lpg_Block;

        IAst* getsynchronized() { return lpg_synchronized; };
        void setsynchronized(IAst* lpg_synchronized) { this->lpg_synchronized = lpg_synchronized; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getBlock() { return lpg_Block; };
        void setBlock(IAst* lpg_Block) { this->lpg_Block = lpg_Block; }

        SynchronizedStatement(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_synchronized,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN,
            IAst* lpg_Block) :Ast(leftIToken, rightIToken) {
            this->lpg_synchronized = lpg_synchronized;
            ((Ast*)lpg_synchronized)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_Block = lpg_Block;
            ((Ast*)lpg_Block)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_synchronized);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_Block);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_synchronized->accept(v);
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
                lpg_Block->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 328:  Catches ::= CatchClause
     *</em>
     *<p>
     *<b>
     *<li>Rule 329:  Catches ::= Catches CatchClause
     *</b>
     */
    struct Catches :public Ast
    {
        IAst* lpg_Catches;
        IAst* lpg_CatchClause;

        IAst* getCatches() { return lpg_Catches; };
        void setCatches(IAst* lpg_Catches) { this->lpg_Catches = lpg_Catches; }
        IAst* getCatchClause() { return lpg_CatchClause; };
        void setCatchClause(IAst* lpg_CatchClause) { this->lpg_CatchClause = lpg_CatchClause; }

        Catches(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Catches,
            IAst* lpg_CatchClause) :Ast(leftIToken, rightIToken) {
            this->lpg_Catches = lpg_Catches;
            ((Ast*)lpg_Catches)->setParent(this);
            this->lpg_CatchClause = lpg_CatchClause;
            ((Ast*)lpg_CatchClause)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Catches);
            list.push_back(lpg_CatchClause);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Catches->accept(v);
                lpg_CatchClause->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 330:  CatchClause ::= catch ( FormalParameter ) Block
     *</b>
     */
    struct CatchClause :public Ast
    {
        IAst* lpg_catch;
        IAst* lpg_LPAREN;
        IAst* lpg_FormalParameter;
        IAst* lpg_RPAREN;
        IAst* lpg_Block;

        IAst* getcatch() { return lpg_catch; };
        void setcatch(IAst* lpg_catch) { this->lpg_catch = lpg_catch; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getFormalParameter() { return lpg_FormalParameter; };
        void setFormalParameter(IAst* lpg_FormalParameter) { this->lpg_FormalParameter = lpg_FormalParameter; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getBlock() { return lpg_Block; };
        void setBlock(IAst* lpg_Block) { this->lpg_Block = lpg_Block; }

        CatchClause(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_catch,
            IAst* lpg_LPAREN,
            IAst* lpg_FormalParameter,
            IAst* lpg_RPAREN,
            IAst* lpg_Block) :Ast(leftIToken, rightIToken) {
            this->lpg_catch = lpg_catch;
            ((Ast*)lpg_catch)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_FormalParameter = lpg_FormalParameter;
            ((Ast*)lpg_FormalParameter)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_Block = lpg_Block;
            ((Ast*)lpg_Block)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_catch);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_FormalParameter);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_Block);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_catch->accept(v);
                lpg_LPAREN->accept(v);
                lpg_FormalParameter->accept(v);
                lpg_RPAREN->accept(v);
                lpg_Block->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 331:  Finally ::= finally Block
     *</b>
     */
    struct Finally :public Ast
    {
        IAst* lpg_finally;
        IAst* lpg_Block;

        IAst* getfinally() { return lpg_finally; };
        void setfinally(IAst* lpg_finally) { this->lpg_finally = lpg_finally; }
        IAst* getBlock() { return lpg_Block; };
        void setBlock(IAst* lpg_Block) { this->lpg_Block = lpg_Block; }

        Finally(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_finally,
            IAst* lpg_Block) :Ast(leftIToken, rightIToken) {
            this->lpg_finally = lpg_finally;
            ((Ast*)lpg_finally)->setParent(this);
            this->lpg_Block = lpg_Block;
            ((Ast*)lpg_Block)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_finally);
            list.push_back(lpg_Block);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_finally->accept(v);
                lpg_Block->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 356:  ArgumentList ::= Expression
     *</em>
     *<p>
     *<b>
     *<li>Rule 357:  ArgumentList ::= ArgumentList , Expression
     *</b>
     */
    struct ArgumentList :public Ast
    {
        IAst* lpg_ArgumentList;
        IAst* lpg_COMMA;
        IAst* lpg_Expression;

        IAst* getArgumentList() { return lpg_ArgumentList; };
        void setArgumentList(IAst* lpg_ArgumentList) { this->lpg_ArgumentList = lpg_ArgumentList; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }

        ArgumentList(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ArgumentList,
            IAst* lpg_COMMA,
            IAst* lpg_Expression) :Ast(leftIToken, rightIToken) {
            this->lpg_ArgumentList = lpg_ArgumentList;
            ((Ast*)lpg_ArgumentList)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ArgumentList);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_Expression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ArgumentList->accept(v);
                lpg_COMMA->accept(v);
                lpg_Expression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 362:  DimExprs ::= DimExpr
     *</em>
     *<p>
     *<b>
     *<li>Rule 363:  DimExprs ::= DimExprs DimExpr
     *</b>
     */
    struct DimExprs :public Ast
    {
        IAst* lpg_DimExprs;
        IAst* lpg_DimExpr;

        IAst* getDimExprs() { return lpg_DimExprs; };
        void setDimExprs(IAst* lpg_DimExprs) { this->lpg_DimExprs = lpg_DimExprs; }
        IAst* getDimExpr() { return lpg_DimExpr; };
        void setDimExpr(IAst* lpg_DimExpr) { this->lpg_DimExpr = lpg_DimExpr; }

        DimExprs(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_DimExprs,
            IAst* lpg_DimExpr) :Ast(leftIToken, rightIToken) {
            this->lpg_DimExprs = lpg_DimExprs;
            ((Ast*)lpg_DimExprs)->setParent(this);
            this->lpg_DimExpr = lpg_DimExpr;
            ((Ast*)lpg_DimExpr)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_DimExprs);
            list.push_back(lpg_DimExpr);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_DimExprs->accept(v);
                lpg_DimExpr->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 364:  DimExpr ::= [ Expression ]
     *</b>
     */
    struct DimExpr :public Ast
    {
        IAst* lpg_LBRACKET;
        IAst* lpg_Expression;
        IAst* lpg_RBRACKET;

        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        DimExpr(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACKET,
            IAst* lpg_Expression,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACKET->accept(v);
                lpg_Expression->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 381:  PostIncrementExpression ::= PostfixExpression ++
     *</b>
     */
    struct PostIncrementExpression :public Ast
    {
        IAst* lpg_PostfixExpression;
        IAst* lpg_PLUS_PLUS;

        IAst* getPostfixExpression() { return lpg_PostfixExpression; };
        void setPostfixExpression(IAst* lpg_PostfixExpression) { this->lpg_PostfixExpression = lpg_PostfixExpression; }
        IAst* getPLUS_PLUS() { return lpg_PLUS_PLUS; };
        void setPLUS_PLUS(IAst* lpg_PLUS_PLUS) { this->lpg_PLUS_PLUS = lpg_PLUS_PLUS; }

        PostIncrementExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PostfixExpression,
            IAst* lpg_PLUS_PLUS) :Ast(leftIToken, rightIToken) {
            this->lpg_PostfixExpression = lpg_PostfixExpression;
            ((Ast*)lpg_PostfixExpression)->setParent(this);
            this->lpg_PLUS_PLUS = lpg_PLUS_PLUS;
            ((Ast*)lpg_PLUS_PLUS)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PostfixExpression);
            list.push_back(lpg_PLUS_PLUS);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PostfixExpression->accept(v);
                lpg_PLUS_PLUS->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 382:  PostDecrementExpression ::= PostfixExpression --
     *</b>
     */
    struct PostDecrementExpression :public Ast
    {
        IAst* lpg_PostfixExpression;
        IAst* lpg_MINUS_MINUS;

        IAst* getPostfixExpression() { return lpg_PostfixExpression; };
        void setPostfixExpression(IAst* lpg_PostfixExpression) { this->lpg_PostfixExpression = lpg_PostfixExpression; }
        IAst* getMINUS_MINUS() { return lpg_MINUS_MINUS; };
        void setMINUS_MINUS(IAst* lpg_MINUS_MINUS) { this->lpg_MINUS_MINUS = lpg_MINUS_MINUS; }

        PostDecrementExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PostfixExpression,
            IAst* lpg_MINUS_MINUS) :Ast(leftIToken, rightIToken) {
            this->lpg_PostfixExpression = lpg_PostfixExpression;
            ((Ast*)lpg_PostfixExpression)->setParent(this);
            this->lpg_MINUS_MINUS = lpg_MINUS_MINUS;
            ((Ast*)lpg_MINUS_MINUS)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PostfixExpression);
            list.push_back(lpg_MINUS_MINUS);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PostfixExpression->accept(v);
                lpg_MINUS_MINUS->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 388:  PreIncrementExpression ::= ++ UnaryExpression
     *</b>
     */
    struct PreIncrementExpression :public Ast
    {
        IAst* lpg_PLUS_PLUS;
        IAst* lpg_UnaryExpression;

        IAst* getPLUS_PLUS() { return lpg_PLUS_PLUS; };
        void setPLUS_PLUS(IAst* lpg_PLUS_PLUS) { this->lpg_PLUS_PLUS = lpg_PLUS_PLUS; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        PreIncrementExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PLUS_PLUS,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_PLUS_PLUS = lpg_PLUS_PLUS;
            ((Ast*)lpg_PLUS_PLUS)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PLUS_PLUS);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PLUS_PLUS->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 389:  PreDecrementExpression ::= -- UnaryExpression
     *</b>
     */
    struct PreDecrementExpression :public Ast
    {
        IAst* lpg_MINUS_MINUS;
        IAst* lpg_UnaryExpression;

        IAst* getMINUS_MINUS() { return lpg_MINUS_MINUS; };
        void setMINUS_MINUS(IAst* lpg_MINUS_MINUS) { this->lpg_MINUS_MINUS = lpg_MINUS_MINUS; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        PreDecrementExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MINUS_MINUS,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_MINUS_MINUS = lpg_MINUS_MINUS;
            ((Ast*)lpg_MINUS_MINUS)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MINUS_MINUS);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MINUS_MINUS->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 416:  AndExpression ::= EqualityExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 417:  AndExpression ::= AndExpression & EqualityExpression
     *</b>
     */
    struct AndExpression :public Ast
    {
        IAst* lpg_AndExpression;
        IAst* lpg_AND;
        IAst* lpg_EqualityExpression;

        IAst* getAndExpression() { return lpg_AndExpression; };
        void setAndExpression(IAst* lpg_AndExpression) { this->lpg_AndExpression = lpg_AndExpression; }
        IAst* getAND() { return lpg_AND; };
        void setAND(IAst* lpg_AND) { this->lpg_AND = lpg_AND; }
        IAst* getEqualityExpression() { return lpg_EqualityExpression; };
        void setEqualityExpression(IAst* lpg_EqualityExpression) { this->lpg_EqualityExpression = lpg_EqualityExpression; }

        AndExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AndExpression,
            IAst* lpg_AND,
            IAst* lpg_EqualityExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_AndExpression = lpg_AndExpression;
            ((Ast*)lpg_AndExpression)->setParent(this);
            this->lpg_AND = lpg_AND;
            ((Ast*)lpg_AND)->setParent(this);
            this->lpg_EqualityExpression = lpg_EqualityExpression;
            ((Ast*)lpg_EqualityExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AndExpression);
            list.push_back(lpg_AND);
            list.push_back(lpg_EqualityExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AndExpression->accept(v);
                lpg_AND->accept(v);
                lpg_EqualityExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 418:  ExclusiveOrExpression ::= AndExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 419:  ExclusiveOrExpression ::= ExclusiveOrExpression ^ AndExpression
     *</b>
     */
    struct ExclusiveOrExpression :public Ast
    {
        IAst* lpg_ExclusiveOrExpression;
        IAst* lpg_XOR;
        IAst* lpg_AndExpression;

        IAst* getExclusiveOrExpression() { return lpg_ExclusiveOrExpression; };
        void setExclusiveOrExpression(IAst* lpg_ExclusiveOrExpression) { this->lpg_ExclusiveOrExpression = lpg_ExclusiveOrExpression; }
        IAst* getXOR() { return lpg_XOR; };
        void setXOR(IAst* lpg_XOR) { this->lpg_XOR = lpg_XOR; }
        IAst* getAndExpression() { return lpg_AndExpression; };
        void setAndExpression(IAst* lpg_AndExpression) { this->lpg_AndExpression = lpg_AndExpression; }

        ExclusiveOrExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ExclusiveOrExpression,
            IAst* lpg_XOR,
            IAst* lpg_AndExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ExclusiveOrExpression = lpg_ExclusiveOrExpression;
            ((Ast*)lpg_ExclusiveOrExpression)->setParent(this);
            this->lpg_XOR = lpg_XOR;
            ((Ast*)lpg_XOR)->setParent(this);
            this->lpg_AndExpression = lpg_AndExpression;
            ((Ast*)lpg_AndExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ExclusiveOrExpression);
            list.push_back(lpg_XOR);
            list.push_back(lpg_AndExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ExclusiveOrExpression->accept(v);
                lpg_XOR->accept(v);
                lpg_AndExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 420:  InclusiveOrExpression ::= ExclusiveOrExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 421:  InclusiveOrExpression ::= InclusiveOrExpression | ExclusiveOrExpression
     *</b>
     */
    struct InclusiveOrExpression :public Ast
    {
        IAst* lpg_InclusiveOrExpression;
        IAst* lpg_OR;
        IAst* lpg_ExclusiveOrExpression;

        IAst* getInclusiveOrExpression() { return lpg_InclusiveOrExpression; };
        void setInclusiveOrExpression(IAst* lpg_InclusiveOrExpression) { this->lpg_InclusiveOrExpression = lpg_InclusiveOrExpression; }
        IAst* getOR() { return lpg_OR; };
        void setOR(IAst* lpg_OR) { this->lpg_OR = lpg_OR; }
        IAst* getExclusiveOrExpression() { return lpg_ExclusiveOrExpression; };
        void setExclusiveOrExpression(IAst* lpg_ExclusiveOrExpression) { this->lpg_ExclusiveOrExpression = lpg_ExclusiveOrExpression; }

        InclusiveOrExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_InclusiveOrExpression,
            IAst* lpg_OR,
            IAst* lpg_ExclusiveOrExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_InclusiveOrExpression = lpg_InclusiveOrExpression;
            ((Ast*)lpg_InclusiveOrExpression)->setParent(this);
            this->lpg_OR = lpg_OR;
            ((Ast*)lpg_OR)->setParent(this);
            this->lpg_ExclusiveOrExpression = lpg_ExclusiveOrExpression;
            ((Ast*)lpg_ExclusiveOrExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_InclusiveOrExpression);
            list.push_back(lpg_OR);
            list.push_back(lpg_ExclusiveOrExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_InclusiveOrExpression->accept(v);
                lpg_OR->accept(v);
                lpg_ExclusiveOrExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 422:  ConditionalAndExpression ::= InclusiveOrExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 423:  ConditionalAndExpression ::= ConditionalAndExpression && InclusiveOrExpression
     *</b>
     */
    struct ConditionalAndExpression :public Ast
    {
        IAst* lpg_ConditionalAndExpression;
        IAst* lpg_AND_AND;
        IAst* lpg_InclusiveOrExpression;

        IAst* getConditionalAndExpression() { return lpg_ConditionalAndExpression; };
        void setConditionalAndExpression(IAst* lpg_ConditionalAndExpression) { this->lpg_ConditionalAndExpression = lpg_ConditionalAndExpression; }
        IAst* getAND_AND() { return lpg_AND_AND; };
        void setAND_AND(IAst* lpg_AND_AND) { this->lpg_AND_AND = lpg_AND_AND; }
        IAst* getInclusiveOrExpression() { return lpg_InclusiveOrExpression; };
        void setInclusiveOrExpression(IAst* lpg_InclusiveOrExpression) { this->lpg_InclusiveOrExpression = lpg_InclusiveOrExpression; }

        ConditionalAndExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConditionalAndExpression,
            IAst* lpg_AND_AND,
            IAst* lpg_InclusiveOrExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ConditionalAndExpression = lpg_ConditionalAndExpression;
            ((Ast*)lpg_ConditionalAndExpression)->setParent(this);
            this->lpg_AND_AND = lpg_AND_AND;
            ((Ast*)lpg_AND_AND)->setParent(this);
            this->lpg_InclusiveOrExpression = lpg_InclusiveOrExpression;
            ((Ast*)lpg_InclusiveOrExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConditionalAndExpression);
            list.push_back(lpg_AND_AND);
            list.push_back(lpg_InclusiveOrExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ConditionalAndExpression->accept(v);
                lpg_AND_AND->accept(v);
                lpg_InclusiveOrExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 424:  ConditionalOrExpression ::= ConditionalAndExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 425:  ConditionalOrExpression ::= ConditionalOrExpression || ConditionalAndExpression
     *</b>
     */
    struct ConditionalOrExpression :public Ast
    {
        IAst* lpg_ConditionalOrExpression;
        IAst* lpg_OR_OR;
        IAst* lpg_ConditionalAndExpression;

        IAst* getConditionalOrExpression() { return lpg_ConditionalOrExpression; };
        void setConditionalOrExpression(IAst* lpg_ConditionalOrExpression) { this->lpg_ConditionalOrExpression = lpg_ConditionalOrExpression; }
        IAst* getOR_OR() { return lpg_OR_OR; };
        void setOR_OR(IAst* lpg_OR_OR) { this->lpg_OR_OR = lpg_OR_OR; }
        IAst* getConditionalAndExpression() { return lpg_ConditionalAndExpression; };
        void setConditionalAndExpression(IAst* lpg_ConditionalAndExpression) { this->lpg_ConditionalAndExpression = lpg_ConditionalAndExpression; }

        ConditionalOrExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConditionalOrExpression,
            IAst* lpg_OR_OR,
            IAst* lpg_ConditionalAndExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ConditionalOrExpression = lpg_ConditionalOrExpression;
            ((Ast*)lpg_ConditionalOrExpression)->setParent(this);
            this->lpg_OR_OR = lpg_OR_OR;
            ((Ast*)lpg_OR_OR)->setParent(this);
            this->lpg_ConditionalAndExpression = lpg_ConditionalAndExpression;
            ((Ast*)lpg_ConditionalAndExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConditionalOrExpression);
            list.push_back(lpg_OR_OR);
            list.push_back(lpg_ConditionalAndExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ConditionalOrExpression->accept(v);
                lpg_OR_OR->accept(v);
                lpg_ConditionalAndExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 426:  ConditionalExpression ::= ConditionalOrExpression
     *</em>
     *<p>
     *<b>
     *<li>Rule 427:  ConditionalExpression ::= ConditionalOrExpression ? Expression : ConditionalExpression
     *</b>
     */
    struct ConditionalExpression :public Ast
    {
        IAst* lpg_ConditionalOrExpression;
        IAst* lpg_QUESTION;
        IAst* lpg_Expression;
        IAst* lpg_COLON;
        IAst* lpg_ConditionalExpression;

        IAst* getConditionalOrExpression() { return lpg_ConditionalOrExpression; };
        void setConditionalOrExpression(IAst* lpg_ConditionalOrExpression) { this->lpg_ConditionalOrExpression = lpg_ConditionalOrExpression; }
        IAst* getQUESTION() { return lpg_QUESTION; };
        void setQUESTION(IAst* lpg_QUESTION) { this->lpg_QUESTION = lpg_QUESTION; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }
        IAst* getConditionalExpression() { return lpg_ConditionalExpression; };
        void setConditionalExpression(IAst* lpg_ConditionalExpression) { this->lpg_ConditionalExpression = lpg_ConditionalExpression; }

        ConditionalExpression(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ConditionalOrExpression,
            IAst* lpg_QUESTION,
            IAst* lpg_Expression,
            IAst* lpg_COLON,
            IAst* lpg_ConditionalExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ConditionalOrExpression = lpg_ConditionalOrExpression;
            ((Ast*)lpg_ConditionalOrExpression)->setParent(this);
            this->lpg_QUESTION = lpg_QUESTION;
            ((Ast*)lpg_QUESTION)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            this->lpg_ConditionalExpression = lpg_ConditionalExpression;
            ((Ast*)lpg_ConditionalExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ConditionalOrExpression);
            list.push_back(lpg_QUESTION);
            list.push_back(lpg_Expression);
            list.push_back(lpg_COLON);
            list.push_back(lpg_ConditionalExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ConditionalOrExpression->accept(v);
                lpg_QUESTION->accept(v);
                lpg_Expression->accept(v);
                lpg_COLON->accept(v);
                lpg_ConditionalExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 430:  Assignment ::= LeftHandSide AssignmentOperator AssignmentExpression
     *</b>
     */
    struct Assignment :public Ast
    {
        IAst* lpg_LeftHandSide;
        IAst* lpg_AssignmentOperator;
        IAst* lpg_AssignmentExpression;

        IAst* getLeftHandSide() { return lpg_LeftHandSide; };
        void setLeftHandSide(IAst* lpg_LeftHandSide) { this->lpg_LeftHandSide = lpg_LeftHandSide; }
        IAst* getAssignmentOperator() { return lpg_AssignmentOperator; };
        void setAssignmentOperator(IAst* lpg_AssignmentOperator) { this->lpg_AssignmentOperator = lpg_AssignmentOperator; }
        IAst* getAssignmentExpression() { return lpg_AssignmentExpression; };
        void setAssignmentExpression(IAst* lpg_AssignmentExpression) { this->lpg_AssignmentExpression = lpg_AssignmentExpression; }

        Assignment(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LeftHandSide,
            IAst* lpg_AssignmentOperator,
            IAst* lpg_AssignmentExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_LeftHandSide = lpg_LeftHandSide;
            ((Ast*)lpg_LeftHandSide)->setParent(this);
            this->lpg_AssignmentOperator = lpg_AssignmentOperator;
            ((Ast*)lpg_AssignmentOperator)->setParent(this);
            this->lpg_AssignmentExpression = lpg_AssignmentExpression;
            ((Ast*)lpg_AssignmentExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LeftHandSide);
            list.push_back(lpg_AssignmentOperator);
            list.push_back(lpg_AssignmentExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LeftHandSide->accept(v);
                lpg_AssignmentOperator->accept(v);
                lpg_AssignmentExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 492:  ,opt ::= $Empty
     *</em>
     *<p>
     *<b>
     *<li>Rule 493:  ,opt ::= ,
     *</b>
     */
    struct Commaopt :public AstToken
    {
        IToken* getCOMMA() { return leftIToken; }

        Commaopt(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<em>
     *<li>Rule 504:  ...opt ::= $Empty
     *</em>
     *<p>
     *<b>
     *<li>Rule 505:  ...opt ::= ...
     *</b>
     */
    struct Ellipsisopt :public AstToken
    {
        IToken* getELLIPSIS() { return leftIToken; }

        Ellipsisopt(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 5:  LPGUserAction ::= $BeginAction BlockStatementsopt $EndAction
     *</b>
     */
    struct LPGUserAction0 :public Ast
    {
        IAst* lpg_BeginAction;
        IAst* lpg_BlockStatementsopt;
        IAst* lpg_EndAction;

        IAst* getBeginAction() { return lpg_BeginAction; };
        void setBeginAction(IAst* lpg_BeginAction) { this->lpg_BeginAction = lpg_BeginAction; }
        /**
         * The value returned by <b>getBlockStatementsopt</b> may be <b>nullptr</b>
         */
        IAst* getBlockStatementsopt() { return lpg_BlockStatementsopt; };
        void setBlockStatementsopt(IAst* lpg_BlockStatementsopt) { this->lpg_BlockStatementsopt = lpg_BlockStatementsopt; }
        IAst* getEndAction() { return lpg_EndAction; };
        void setEndAction(IAst* lpg_EndAction) { this->lpg_EndAction = lpg_EndAction; }

        LPGUserAction0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_BeginAction,
            IAst* lpg_BlockStatementsopt,
            IAst* lpg_EndAction) :Ast(leftIToken, rightIToken) {
            this->lpg_BeginAction = lpg_BeginAction;
            ((Ast*)lpg_BeginAction)->setParent(this);
            this->lpg_BlockStatementsopt = lpg_BlockStatementsopt;
            if (lpg_BlockStatementsopt != nullptr) ((Ast*)lpg_BlockStatementsopt)->setParent(this);
            this->lpg_EndAction = lpg_EndAction;
            ((Ast*)lpg_EndAction)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_BeginAction);
            list.push_back(lpg_BlockStatementsopt);
            list.push_back(lpg_EndAction);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_BeginAction->accept(v);
                if (lpg_BlockStatementsopt != nullptr) lpg_BlockStatementsopt->accept(v);
                lpg_EndAction->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 6:  LPGUserAction ::= $BeginJava BlockStatementsopt $EndJava
     *</b>
     */
    struct LPGUserAction1 :public Ast
    {
        IAst* lpg_BeginJava;
        IAst* lpg_BlockStatementsopt;
        IAst* lpg_EndJava;

        IAst* getBeginJava() { return lpg_BeginJava; };
        void setBeginJava(IAst* lpg_BeginJava) { this->lpg_BeginJava = lpg_BeginJava; }
        /**
         * The value returned by <b>getBlockStatementsopt</b> may be <b>nullptr</b>
         */
        IAst* getBlockStatementsopt() { return lpg_BlockStatementsopt; };
        void setBlockStatementsopt(IAst* lpg_BlockStatementsopt) { this->lpg_BlockStatementsopt = lpg_BlockStatementsopt; }
        IAst* getEndJava() { return lpg_EndJava; };
        void setEndJava(IAst* lpg_EndJava) { this->lpg_EndJava = lpg_EndJava; }

        LPGUserAction1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_BeginJava,
            IAst* lpg_BlockStatementsopt,
            IAst* lpg_EndJava) :Ast(leftIToken, rightIToken) {
            this->lpg_BeginJava = lpg_BeginJava;
            ((Ast*)lpg_BeginJava)->setParent(this);
            this->lpg_BlockStatementsopt = lpg_BlockStatementsopt;
            if (lpg_BlockStatementsopt != nullptr) ((Ast*)lpg_BlockStatementsopt)->setParent(this);
            this->lpg_EndJava = lpg_EndJava;
            ((Ast*)lpg_EndJava)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_BeginJava);
            list.push_back(lpg_BlockStatementsopt);
            list.push_back(lpg_EndJava);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_BeginJava->accept(v);
                if (lpg_BlockStatementsopt != nullptr) lpg_BlockStatementsopt->accept(v);
                lpg_EndJava->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 7:  LPGUserAction ::= $NoAction
     *</b>
     */
    struct LPGUserAction2 :public AstToken
    {
        IToken* getNoAction() { return leftIToken; }

        LPGUserAction2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 8:  LPGUserAction ::= $NullAction
     *</b>
     */
    struct LPGUserAction3 :public AstToken
    {
        IToken* getNullAction() { return leftIToken; }

        LPGUserAction3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 9:  LPGUserAction ::= $BadAction
     *</b>
     */
    struct LPGUserAction4 :public AstToken
    {
        IToken* getBadAction() { return leftIToken; }

        LPGUserAction4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 16:  IntegralType ::= byte
     *</b>
     */
    struct IntegralType0 :public AstToken
    {
        IToken* getbyte() { return leftIToken; }

        IntegralType0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 17:  IntegralType ::= short
     *</b>
     */
    struct IntegralType1 :public AstToken
    {
        IToken* getshort() { return leftIToken; }

        IntegralType1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 18:  IntegralType ::= int
     *</b>
     */
    struct IntegralType2 :public AstToken
    {
        IToken* getint() { return leftIToken; }

        IntegralType2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 19:  IntegralType ::= long
     *</b>
     */
    struct IntegralType3 :public AstToken
    {
        IToken* getlong() { return leftIToken; }

        IntegralType3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 20:  IntegralType ::= char
     *</b>
     */
    struct IntegralType4 :public AstToken
    {
        IToken* getchar() { return leftIToken; }

        IntegralType4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 21:  FloatingPointType ::= float
     *</b>
     */
    struct FloatingPointType0 :public AstToken
    {
        IToken* getfloat() { return leftIToken; }

        FloatingPointType0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 22:  FloatingPointType ::= double
     *</b>
     */
    struct FloatingPointType1 :public AstToken
    {
        IToken* getdouble() { return leftIToken; }

        FloatingPointType1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 45:  WildcardBounds ::= extends ReferenceType
     *</b>
     */
    struct WildcardBounds0 :public Ast
    {
        IAst* lpg_extends;
        IAst* lpg_ReferenceType;

        IAst* getextends() { return lpg_extends; };
        void setextends(IAst* lpg_extends) { this->lpg_extends = lpg_extends; }
        IAst* getReferenceType() { return lpg_ReferenceType; };
        void setReferenceType(IAst* lpg_ReferenceType) { this->lpg_ReferenceType = lpg_ReferenceType; }

        WildcardBounds0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_extends,
            IAst* lpg_ReferenceType) :Ast(leftIToken, rightIToken) {
            this->lpg_extends = lpg_extends;
            ((Ast*)lpg_extends)->setParent(this);
            this->lpg_ReferenceType = lpg_ReferenceType;
            ((Ast*)lpg_ReferenceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_extends);
            list.push_back(lpg_ReferenceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_extends->accept(v);
                lpg_ReferenceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 46:  WildcardBounds ::= super ReferenceType
     *</b>
     */
    struct WildcardBounds1 :public Ast
    {
        IAst* lpg_super;
        IAst* lpg_ReferenceType;

        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getReferenceType() { return lpg_ReferenceType; };
        void setReferenceType(IAst* lpg_ReferenceType) { this->lpg_ReferenceType = lpg_ReferenceType; }

        WildcardBounds1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_super,
            IAst* lpg_ReferenceType) :Ast(leftIToken, rightIToken) {
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_ReferenceType = lpg_ReferenceType;
            ((Ast*)lpg_ReferenceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_super);
            list.push_back(lpg_ReferenceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_super->accept(v);
                lpg_ReferenceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 80:  ClassModifier ::= public
     *</b>
     */
    struct ClassModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        ClassModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 81:  ClassModifier ::= protected
     *</b>
     */
    struct ClassModifier1 :public AstToken
    {
        IToken* getprotected() { return leftIToken; }

        ClassModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 82:  ClassModifier ::= private
     *</b>
     */
    struct ClassModifier2 :public AstToken
    {
        IToken* getprivate() { return leftIToken; }

        ClassModifier2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 83:  ClassModifier ::= abstract
     *</b>
     */
    struct ClassModifier3 :public AstToken
    {
        IToken* getabstract() { return leftIToken; }

        ClassModifier3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 84:  ClassModifier ::= static
     *</b>
     */
    struct ClassModifier4 :public AstToken
    {
        IToken* getstatic() { return leftIToken; }

        ClassModifier4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 85:  ClassModifier ::= final
     *</b>
     */
    struct ClassModifier5 :public AstToken
    {
        IToken* getfinal() { return leftIToken; }

        ClassModifier5(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 86:  ClassModifier ::= strictfp
     *</b>
     */
    struct ClassModifier6 :public AstToken
    {
        IToken* getstrictfp() { return leftIToken; }

        ClassModifier6(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 118:  FieldModifier ::= public
     *</b>
     */
    struct FieldModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        FieldModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 119:  FieldModifier ::= protected
     *</b>
     */
    struct FieldModifier1 :public AstToken
    {
        IToken* getprotected() { return leftIToken; }

        FieldModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 120:  FieldModifier ::= private
     *</b>
     */
    struct FieldModifier2 :public AstToken
    {
        IToken* getprivate() { return leftIToken; }

        FieldModifier2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 121:  FieldModifier ::= static
     *</b>
     */
    struct FieldModifier3 :public AstToken
    {
        IToken* getstatic() { return leftIToken; }

        FieldModifier3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 122:  FieldModifier ::= final
     *</b>
     */
    struct FieldModifier4 :public AstToken
    {
        IToken* getfinal() { return leftIToken; }

        FieldModifier4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 123:  FieldModifier ::= transient
     *</b>
     */
    struct FieldModifier5 :public AstToken
    {
        IToken* gettransient() { return leftIToken; }

        FieldModifier5(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 124:  FieldModifier ::= volatile
     *</b>
     */
    struct FieldModifier6 :public AstToken
    {
        IToken* getvolatile() { return leftIToken; }

        FieldModifier6(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 129:  MethodDeclarator ::= identifier ( FormalParameterListopt )
     *</b>
     */
    struct MethodDeclarator0 :public Ast
    {
        IAst* lpg_identifier;
        IAst* lpg_LPAREN;
        IAst* lpg_FormalParameterListopt;
        IAst* lpg_RPAREN;

        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getFormalParameterListopt</b> may be <b>nullptr</b>
         */
        IAst* getFormalParameterListopt() { return lpg_FormalParameterListopt; };
        void setFormalParameterListopt(IAst* lpg_FormalParameterListopt) { this->lpg_FormalParameterListopt = lpg_FormalParameterListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        MethodDeclarator0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_identifier,
            IAst* lpg_LPAREN,
            IAst* lpg_FormalParameterListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_FormalParameterListopt = lpg_FormalParameterListopt;
            if (lpg_FormalParameterListopt != nullptr) ((Ast*)lpg_FormalParameterListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_identifier);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_FormalParameterListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_identifier->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_FormalParameterListopt != nullptr) lpg_FormalParameterListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 130:  MethodDeclarator ::= MethodDeclarator [ ]
     *</b>
     */
    struct MethodDeclarator1 :public Ast
    {
        IAst* lpg_MethodDeclarator;
        IAst* lpg_LBRACKET;
        IAst* lpg_RBRACKET;

        IAst* getMethodDeclarator() { return lpg_MethodDeclarator; };
        void setMethodDeclarator(IAst* lpg_MethodDeclarator) { this->lpg_MethodDeclarator = lpg_MethodDeclarator; }
        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        MethodDeclarator1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MethodDeclarator,
            IAst* lpg_LBRACKET,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_MethodDeclarator = lpg_MethodDeclarator;
            ((Ast*)lpg_MethodDeclarator)->setParent(this);
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MethodDeclarator);
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MethodDeclarator->accept(v);
                lpg_LBRACKET->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 144:  MethodModifier ::= public
     *</b>
     */
    struct MethodModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        MethodModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 145:  MethodModifier ::= protected
     *</b>
     */
    struct MethodModifier1 :public AstToken
    {
        IToken* getprotected() { return leftIToken; }

        MethodModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 146:  MethodModifier ::= private
     *</b>
     */
    struct MethodModifier2 :public AstToken
    {
        IToken* getprivate() { return leftIToken; }

        MethodModifier2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 147:  MethodModifier ::= abstract
     *</b>
     */
    struct MethodModifier3 :public AstToken
    {
        IToken* getabstract() { return leftIToken; }

        MethodModifier3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 148:  MethodModifier ::= static
     *</b>
     */
    struct MethodModifier4 :public AstToken
    {
        IToken* getstatic() { return leftIToken; }

        MethodModifier4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 149:  MethodModifier ::= final
     *</b>
     */
    struct MethodModifier5 :public AstToken
    {
        IToken* getfinal() { return leftIToken; }

        MethodModifier5(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 150:  MethodModifier ::= synchronized
     *</b>
     */
    struct MethodModifier6 :public AstToken
    {
        IToken* getsynchronized() { return leftIToken; }

        MethodModifier6(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 151:  MethodModifier ::= native
     *</b>
     */
    struct MethodModifier7 :public AstToken
    {
        IToken* getnative() { return leftIToken; }

        MethodModifier7(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 152:  MethodModifier ::= strictfp
     *</b>
     */
    struct MethodModifier8 :public AstToken
    {
        IToken* getstrictfp() { return leftIToken; }

        MethodModifier8(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 168:  ConstructorModifier ::= public
     *</b>
     */
    struct ConstructorModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        ConstructorModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 169:  ConstructorModifier ::= protected
     *</b>
     */
    struct ConstructorModifier1 :public AstToken
    {
        IToken* getprotected() { return leftIToken; }

        ConstructorModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 170:  ConstructorModifier ::= private
     *</b>
     */
    struct ConstructorModifier2 :public AstToken
    {
        IToken* getprivate() { return leftIToken; }

        ConstructorModifier2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 172:  ExplicitConstructorInvocation ::= TypeArgumentsopt this ( ArgumentListopt ) ;
     *</b>
     */
    struct ExplicitConstructorInvocation0 :public Ast
    {
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_this;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;
        IAst* lpg_SEMICOLON;

        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getthis() { return lpg_this; };
        void setthis(IAst* lpg_this) { this->lpg_this = lpg_this; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ExplicitConstructorInvocation0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_this,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_this = lpg_this;
            ((Ast*)lpg_this)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_this);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_this->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 173:  ExplicitConstructorInvocation ::= TypeArgumentsopt super ( ArgumentListopt ) ;
     *</b>
     */
    struct ExplicitConstructorInvocation1 :public Ast
    {
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_super;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;
        IAst* lpg_SEMICOLON;

        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ExplicitConstructorInvocation1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_super,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_super);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_super->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 174:  ExplicitConstructorInvocation ::= Primary . TypeArgumentsopt super ( ArgumentListopt ) ;
     *</b>
     */
    struct ExplicitConstructorInvocation2 :public Ast
    {
        IAst* lpg_Primary;
        IAst* lpg_DOT;
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_super;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;
        IAst* lpg_SEMICOLON;

        IAst* getPrimary() { return lpg_Primary; };
        void setPrimary(IAst* lpg_Primary) { this->lpg_Primary = lpg_Primary; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        ExplicitConstructorInvocation2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Primary,
            IAst* lpg_DOT,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_super,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_Primary = lpg_Primary;
            ((Ast*)lpg_Primary)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Primary);
            list.push_back(lpg_DOT);
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_super);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Primary->accept(v);
                lpg_DOT->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_super->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 188:  InterfaceModifier ::= public
     *</b>
     */
    struct InterfaceModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        InterfaceModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 189:  InterfaceModifier ::= protected
     *</b>
     */
    struct InterfaceModifier1 :public AstToken
    {
        IToken* getprotected() { return leftIToken; }

        InterfaceModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 190:  InterfaceModifier ::= private
     *</b>
     */
    struct InterfaceModifier2 :public AstToken
    {
        IToken* getprivate() { return leftIToken; }

        InterfaceModifier2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 191:  InterfaceModifier ::= abstract
     *</b>
     */
    struct InterfaceModifier3 :public AstToken
    {
        IToken* getabstract() { return leftIToken; }

        InterfaceModifier3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 192:  InterfaceModifier ::= static
     *</b>
     */
    struct InterfaceModifier4 :public AstToken
    {
        IToken* getstatic() { return leftIToken; }

        InterfaceModifier4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 193:  InterfaceModifier ::= strictfp
     *</b>
     */
    struct InterfaceModifier5 :public AstToken
    {
        IToken* getstrictfp() { return leftIToken; }

        InterfaceModifier5(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 194:  ExtendsInterfaces ::= extends InterfaceType
     *</b>
     */
    struct ExtendsInterfaces0 :public Ast
    {
        IAst* lpg_extends;
        IAst* lpg_InterfaceType;

        IAst* getextends() { return lpg_extends; };
        void setextends(IAst* lpg_extends) { this->lpg_extends = lpg_extends; }
        IAst* getInterfaceType() { return lpg_InterfaceType; };
        void setInterfaceType(IAst* lpg_InterfaceType) { this->lpg_InterfaceType = lpg_InterfaceType; }

        ExtendsInterfaces0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_extends,
            IAst* lpg_InterfaceType) :Ast(leftIToken, rightIToken) {
            this->lpg_extends = lpg_extends;
            ((Ast*)lpg_extends)->setParent(this);
            this->lpg_InterfaceType = lpg_InterfaceType;
            ((Ast*)lpg_InterfaceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_extends);
            list.push_back(lpg_InterfaceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_extends->accept(v);
                lpg_InterfaceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 195:  ExtendsInterfaces ::= ExtendsInterfaces , InterfaceType
     *</b>
     */
    struct ExtendsInterfaces1 :public Ast
    {
        IAst* lpg_ExtendsInterfaces;
        IAst* lpg_COMMA;
        IAst* lpg_InterfaceType;

        IAst* getExtendsInterfaces() { return lpg_ExtendsInterfaces; };
        void setExtendsInterfaces(IAst* lpg_ExtendsInterfaces) { this->lpg_ExtendsInterfaces = lpg_ExtendsInterfaces; }
        IAst* getCOMMA() { return lpg_COMMA; };
        void setCOMMA(IAst* lpg_COMMA) { this->lpg_COMMA = lpg_COMMA; }
        IAst* getInterfaceType() { return lpg_InterfaceType; };
        void setInterfaceType(IAst* lpg_InterfaceType) { this->lpg_InterfaceType = lpg_InterfaceType; }

        ExtendsInterfaces1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ExtendsInterfaces,
            IAst* lpg_COMMA,
            IAst* lpg_InterfaceType) :Ast(leftIToken, rightIToken) {
            this->lpg_ExtendsInterfaces = lpg_ExtendsInterfaces;
            ((Ast*)lpg_ExtendsInterfaces)->setParent(this);
            this->lpg_COMMA = lpg_COMMA;
            ((Ast*)lpg_COMMA)->setParent(this);
            this->lpg_InterfaceType = lpg_InterfaceType;
            ((Ast*)lpg_InterfaceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ExtendsInterfaces);
            list.push_back(lpg_COMMA);
            list.push_back(lpg_InterfaceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ExtendsInterfaces->accept(v);
                lpg_COMMA->accept(v);
                lpg_InterfaceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 208:  ConstantModifier ::= public
     *</b>
     */
    struct ConstantModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        ConstantModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 209:  ConstantModifier ::= static
     *</b>
     */
    struct ConstantModifier1 :public AstToken
    {
        IToken* getstatic() { return leftIToken; }

        ConstantModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 210:  ConstantModifier ::= final
     *</b>
     */
    struct ConstantModifier2 :public AstToken
    {
        IToken* getfinal() { return leftIToken; }

        ConstantModifier2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 215:  AbstractMethodModifier ::= public
     *</b>
     */
    struct AbstractMethodModifier0 :public AstToken
    {
        IToken* getpublic() { return leftIToken; }

        AbstractMethodModifier0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 216:  AbstractMethodModifier ::= abstract
     *</b>
     */
    struct AbstractMethodModifier1 :public AstToken
    {
        IToken* getabstract() { return leftIToken; }

        AbstractMethodModifier1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 221:  AnnotationTypeElementDeclaration ::= AbstractMethodModifiersopt Type identifier ( ) DefaultValueopt ;
     *</b>
     */
    struct AnnotationTypeElementDeclaration0 :public Ast
    {
        IAst* lpg_AbstractMethodModifiersopt;
        IAst* lpg_Type;
        IAst* lpg_identifier;
        IAst* lpg_LPAREN;
        IAst* lpg_RPAREN;
        IAst* lpg_DefaultValueopt;
        IAst* lpg_SEMICOLON;

        /**
         * The value returned by <b>getAbstractMethodModifiersopt</b> may be <b>nullptr</b>
         */
        IAst* getAbstractMethodModifiersopt() { return lpg_AbstractMethodModifiersopt; };
        void setAbstractMethodModifiersopt(IAst* lpg_AbstractMethodModifiersopt) { this->lpg_AbstractMethodModifiersopt = lpg_AbstractMethodModifiersopt; }
        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        /**
         * The value returned by <b>getDefaultValueopt</b> may be <b>nullptr</b>
         */
        IAst* getDefaultValueopt() { return lpg_DefaultValueopt; };
        void setDefaultValueopt(IAst* lpg_DefaultValueopt) { this->lpg_DefaultValueopt = lpg_DefaultValueopt; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        AnnotationTypeElementDeclaration0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AbstractMethodModifiersopt,
            IAst* lpg_Type,
            IAst* lpg_identifier,
            IAst* lpg_LPAREN,
            IAst* lpg_RPAREN,
            IAst* lpg_DefaultValueopt,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_AbstractMethodModifiersopt = lpg_AbstractMethodModifiersopt;
            if (lpg_AbstractMethodModifiersopt != nullptr) ((Ast*)lpg_AbstractMethodModifiersopt)->setParent(this);
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_DefaultValueopt = lpg_DefaultValueopt;
            if (lpg_DefaultValueopt != nullptr) ((Ast*)lpg_DefaultValueopt)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AbstractMethodModifiersopt);
            list.push_back(lpg_Type);
            list.push_back(lpg_identifier);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_DefaultValueopt);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                if (lpg_AbstractMethodModifiersopt != nullptr) lpg_AbstractMethodModifiersopt->accept(v);
                lpg_Type->accept(v);
                lpg_identifier->accept(v);
                lpg_LPAREN->accept(v);
                lpg_RPAREN->accept(v);
                if (lpg_DefaultValueopt != nullptr) lpg_DefaultValueopt->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 227:  AnnotationTypeElementDeclaration ::= ;
     *</b>
     */
    struct AnnotationTypeElementDeclaration1 :public AstToken
    {
        IToken* getSEMICOLON() { return leftIToken; }

        AnnotationTypeElementDeclaration1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 295:  AssertStatement ::= assert Expression ;
     *</b>
     */
    struct AssertStatement0 :public Ast
    {
        IAst* lpg_assert;
        IAst* lpg_Expression;
        IAst* lpg_SEMICOLON;

        IAst* getassert() { return lpg_assert; };
        void setassert(IAst* lpg_assert) { this->lpg_assert = lpg_assert; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        AssertStatement0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_assert,
            IAst* lpg_Expression,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_assert = lpg_assert;
            ((Ast*)lpg_assert)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_assert);
            list.push_back(lpg_Expression);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_assert->accept(v);
                lpg_Expression->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 296:  AssertStatement ::= assert Expression : Expression ;
     *</b>
     */
    struct AssertStatement1 :public Ast
    {
        IAst* lpg_assert;
        IAst* lpg_Expression;
        IAst* lpg_COLON;
        IAst* lpg_Expression4;
        IAst* lpg_SEMICOLON;

        IAst* getassert() { return lpg_assert; };
        void setassert(IAst* lpg_assert) { this->lpg_assert = lpg_assert; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }
        IAst* getExpression4() { return lpg_Expression4; };
        void setExpression4(IAst* lpg_Expression4) { this->lpg_Expression4 = lpg_Expression4; }
        IAst* getSEMICOLON() { return lpg_SEMICOLON; };
        void setSEMICOLON(IAst* lpg_SEMICOLON) { this->lpg_SEMICOLON = lpg_SEMICOLON; }

        AssertStatement1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_assert,
            IAst* lpg_Expression,
            IAst* lpg_COLON,
            IAst* lpg_Expression4,
            IAst* lpg_SEMICOLON) :Ast(leftIToken, rightIToken) {
            this->lpg_assert = lpg_assert;
            ((Ast*)lpg_assert)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            this->lpg_Expression4 = lpg_Expression4;
            ((Ast*)lpg_Expression4)->setParent(this);
            this->lpg_SEMICOLON = lpg_SEMICOLON;
            ((Ast*)lpg_SEMICOLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_assert);
            list.push_back(lpg_Expression);
            list.push_back(lpg_COLON);
            list.push_back(lpg_Expression4);
            list.push_back(lpg_SEMICOLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_assert->accept(v);
                lpg_Expression->accept(v);
                lpg_COLON->accept(v);
                lpg_Expression4->accept(v);
                lpg_SEMICOLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 304:  SwitchLabel ::= case ConstantExpression :
     *</b>
     */
    struct SwitchLabel0 :public Ast
    {
        IAst* lpg_case;
        IAst* lpg_ConstantExpression;
        IAst* lpg_COLON;

        IAst* getcase() { return lpg_case; };
        void setcase(IAst* lpg_case) { this->lpg_case = lpg_case; }
        IAst* getConstantExpression() { return lpg_ConstantExpression; };
        void setConstantExpression(IAst* lpg_ConstantExpression) { this->lpg_ConstantExpression = lpg_ConstantExpression; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }

        SwitchLabel0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_case,
            IAst* lpg_ConstantExpression,
            IAst* lpg_COLON) :Ast(leftIToken, rightIToken) {
            this->lpg_case = lpg_case;
            ((Ast*)lpg_case)->setParent(this);
            this->lpg_ConstantExpression = lpg_ConstantExpression;
            ((Ast*)lpg_ConstantExpression)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_case);
            list.push_back(lpg_ConstantExpression);
            list.push_back(lpg_COLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_case->accept(v);
                lpg_ConstantExpression->accept(v);
                lpg_COLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 305:  SwitchLabel ::= case EnumConstant :
     *</b>
     */
    struct SwitchLabel1 :public Ast
    {
        IAst* lpg_case;
        IAst* lpg_EnumConstant;
        IAst* lpg_COLON;

        IAst* getcase() { return lpg_case; };
        void setcase(IAst* lpg_case) { this->lpg_case = lpg_case; }
        IAst* getEnumConstant() { return lpg_EnumConstant; };
        void setEnumConstant(IAst* lpg_EnumConstant) { this->lpg_EnumConstant = lpg_EnumConstant; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }

        SwitchLabel1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_case,
            IAst* lpg_EnumConstant,
            IAst* lpg_COLON) :Ast(leftIToken, rightIToken) {
            this->lpg_case = lpg_case;
            ((Ast*)lpg_case)->setParent(this);
            this->lpg_EnumConstant = lpg_EnumConstant;
            ((Ast*)lpg_EnumConstant)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_case);
            list.push_back(lpg_EnumConstant);
            list.push_back(lpg_COLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_case->accept(v);
                lpg_EnumConstant->accept(v);
                lpg_COLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 306:  SwitchLabel ::= default :
     *</b>
     */
    struct SwitchLabel2 :public Ast
    {
        IAst* lpg_default;
        IAst* lpg_COLON;

        IAst* getdefault() { return lpg_default; };
        void setdefault(IAst* lpg_default) { this->lpg_default = lpg_default; }
        IAst* getCOLON() { return lpg_COLON; };
        void setCOLON(IAst* lpg_COLON) { this->lpg_COLON = lpg_COLON; }

        SwitchLabel2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_default,
            IAst* lpg_COLON) :Ast(leftIToken, rightIToken) {
            this->lpg_default = lpg_default;
            ((Ast*)lpg_default)->setParent(this);
            this->lpg_COLON = lpg_COLON;
            ((Ast*)lpg_COLON)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_default);
            list.push_back(lpg_COLON);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_default->accept(v);
                lpg_COLON->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 326:  TryStatement ::= try Block Catches
     *</b>
     */
    struct TryStatement0 :public Ast
    {
        IAst* lpg_try;
        IAst* lpg_Block;
        IAst* lpg_Catches;

        IAst* gettry() { return lpg_try; };
        void settry(IAst* lpg_try) { this->lpg_try = lpg_try; }
        IAst* getBlock() { return lpg_Block; };
        void setBlock(IAst* lpg_Block) { this->lpg_Block = lpg_Block; }
        IAst* getCatches() { return lpg_Catches; };
        void setCatches(IAst* lpg_Catches) { this->lpg_Catches = lpg_Catches; }

        TryStatement0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_try,
            IAst* lpg_Block,
            IAst* lpg_Catches) :Ast(leftIToken, rightIToken) {
            this->lpg_try = lpg_try;
            ((Ast*)lpg_try)->setParent(this);
            this->lpg_Block = lpg_Block;
            ((Ast*)lpg_Block)->setParent(this);
            this->lpg_Catches = lpg_Catches;
            ((Ast*)lpg_Catches)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_try);
            list.push_back(lpg_Block);
            list.push_back(lpg_Catches);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_try->accept(v);
                lpg_Block->accept(v);
                lpg_Catches->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 327:  TryStatement ::= try Block Catchesopt Finally
     *</b>
     */
    struct TryStatement1 :public Ast
    {
        IAst* lpg_try;
        IAst* lpg_Block;
        IAst* lpg_Catchesopt;
        IAst* lpg_Finally;

        IAst* gettry() { return lpg_try; };
        void settry(IAst* lpg_try) { this->lpg_try = lpg_try; }
        IAst* getBlock() { return lpg_Block; };
        void setBlock(IAst* lpg_Block) { this->lpg_Block = lpg_Block; }
        /**
         * The value returned by <b>getCatchesopt</b> may be <b>nullptr</b>
         */
        IAst* getCatchesopt() { return lpg_Catchesopt; };
        void setCatchesopt(IAst* lpg_Catchesopt) { this->lpg_Catchesopt = lpg_Catchesopt; }
        IAst* getFinally() { return lpg_Finally; };
        void setFinally(IAst* lpg_Finally) { this->lpg_Finally = lpg_Finally; }

        TryStatement1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_try,
            IAst* lpg_Block,
            IAst* lpg_Catchesopt,
            IAst* lpg_Finally) :Ast(leftIToken, rightIToken) {
            this->lpg_try = lpg_try;
            ((Ast*)lpg_try)->setParent(this);
            this->lpg_Block = lpg_Block;
            ((Ast*)lpg_Block)->setParent(this);
            this->lpg_Catchesopt = lpg_Catchesopt;
            if (lpg_Catchesopt != nullptr) ((Ast*)lpg_Catchesopt)->setParent(this);
            this->lpg_Finally = lpg_Finally;
            ((Ast*)lpg_Finally)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_try);
            list.push_back(lpg_Block);
            list.push_back(lpg_Catchesopt);
            list.push_back(lpg_Finally);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_try->accept(v);
                lpg_Block->accept(v);
                if (lpg_Catchesopt != nullptr) lpg_Catchesopt->accept(v);
                lpg_Finally->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 335:  PrimaryNoNewArray ::= Type . class
     *</b>
     */
    struct PrimaryNoNewArray0 :public Ast
    {
        IAst* lpg_Type;
        IAst* lpg_DOT;
        IAst* lpg_class;

        IAst* getType() { return lpg_Type; };
        void setType(IAst* lpg_Type) { this->lpg_Type = lpg_Type; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getclass() { return lpg_class; };
        void setclass(IAst* lpg_class) { this->lpg_class = lpg_class; }

        PrimaryNoNewArray0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Type,
            IAst* lpg_DOT,
            IAst* lpg_class) :Ast(leftIToken, rightIToken) {
            this->lpg_Type = lpg_Type;
            ((Ast*)lpg_Type)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_class = lpg_class;
            ((Ast*)lpg_class)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Type);
            list.push_back(lpg_DOT);
            list.push_back(lpg_class);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Type->accept(v);
                lpg_DOT->accept(v);
                lpg_class->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 336:  PrimaryNoNewArray ::= void . class
     *</b>
     */
    struct PrimaryNoNewArray1 :public Ast
    {
        IAst* lpg_void;
        IAst* lpg_DOT;
        IAst* lpg_class;

        IAst* getvoid() { return lpg_void; };
        void setvoid(IAst* lpg_void) { this->lpg_void = lpg_void; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getclass() { return lpg_class; };
        void setclass(IAst* lpg_class) { this->lpg_class = lpg_class; }

        PrimaryNoNewArray1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_void,
            IAst* lpg_DOT,
            IAst* lpg_class) :Ast(leftIToken, rightIToken) {
            this->lpg_void = lpg_void;
            ((Ast*)lpg_void)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_class = lpg_class;
            ((Ast*)lpg_class)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_void);
            list.push_back(lpg_DOT);
            list.push_back(lpg_class);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_void->accept(v);
                lpg_DOT->accept(v);
                lpg_class->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 337:  PrimaryNoNewArray ::= this
     *</b>
     */
    struct PrimaryNoNewArray2 :public AstToken
    {
        IToken* getthis() { return leftIToken; }

        PrimaryNoNewArray2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 338:  PrimaryNoNewArray ::= ClassName . this
     *</b>
     */
    struct PrimaryNoNewArray3 :public Ast
    {
        IAst* lpg_ClassName;
        IAst* lpg_DOT;
        IAst* lpg_this;

        IAst* getClassName() { return lpg_ClassName; };
        void setClassName(IAst* lpg_ClassName) { this->lpg_ClassName = lpg_ClassName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getthis() { return lpg_this; };
        void setthis(IAst* lpg_this) { this->lpg_this = lpg_this; }

        PrimaryNoNewArray3(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassName,
            IAst* lpg_DOT,
            IAst* lpg_this) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassName = lpg_ClassName;
            ((Ast*)lpg_ClassName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_this = lpg_this;
            ((Ast*)lpg_this)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_this);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ClassName->accept(v);
                lpg_DOT->accept(v);
                lpg_this->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 339:  PrimaryNoNewArray ::= ( Expression )
     *</b>
     */
    struct PrimaryNoNewArray4 :public Ast
    {
        IAst* lpg_LPAREN;
        IAst* lpg_Expression;
        IAst* lpg_RPAREN;

        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        PrimaryNoNewArray4(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LPAREN,
            IAst* lpg_Expression,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LPAREN->accept(v);
                lpg_Expression->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 344:  Literal ::= IntegerLiteral
     *</b>
     */
    struct Literal0 :public AstToken
    {
        IToken* getIntegerLiteral() { return leftIToken; }

        Literal0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 345:  Literal ::= LongLiteral
     *</b>
     */
    struct Literal1 :public AstToken
    {
        IToken* getLongLiteral() { return leftIToken; }

        Literal1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 346:  Literal ::= FloatingPointLiteral
     *</b>
     */
    struct Literal2 :public AstToken
    {
        IToken* getFloatingPointLiteral() { return leftIToken; }

        Literal2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 347:  Literal ::= DoubleLiteral
     *</b>
     */
    struct Literal3 :public AstToken
    {
        IToken* getDoubleLiteral() { return leftIToken; }

        Literal3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 349:  Literal ::= CharacterLiteral
     *</b>
     */
    struct Literal4 :public AstToken
    {
        IToken* getCharacterLiteral() { return leftIToken; }

        Literal4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 350:  Literal ::= StringLiteral
     *</b>
     */
    struct Literal5 :public AstToken
    {
        IToken* getStringLiteral() { return leftIToken; }

        Literal5(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 351:  Literal ::= null
     *</b>
     */
    struct Literal6 :public AstToken
    {
        IToken* getnull() { return leftIToken; }

        Literal6(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 352:  BooleanLiteral ::= true
     *</b>
     */
    struct BooleanLiteral0 :public AstToken
    {
        IToken* gettrue() { return leftIToken; }

        BooleanLiteral0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 353:  BooleanLiteral ::= false
     *</b>
     */
    struct BooleanLiteral1 :public AstToken
    {
        IToken* getfalse() { return leftIToken; }

        BooleanLiteral1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 354:  ClassInstanceCreationExpression ::= new TypeArgumentsopt ClassOrInterfaceType TypeArgumentsopt ( ArgumentListopt ) ClassBodyopt
     *</b>
     */
    struct ClassInstanceCreationExpression0 :public Ast
    {
        IAst* lpg_new;
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_ClassOrInterfaceType;
        IAst* lpg_TypeArgumentsopt4;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;
        IAst* lpg_ClassBodyopt;

        IAst* getnew() { return lpg_new; };
        void setnew(IAst* lpg_new) { this->lpg_new = lpg_new; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getClassOrInterfaceType() { return lpg_ClassOrInterfaceType; };
        void setClassOrInterfaceType(IAst* lpg_ClassOrInterfaceType) { this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType; }
        /**
         * The value returned by <b>getTypeArgumentsopt4</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt4() { return lpg_TypeArgumentsopt4; };
        void setTypeArgumentsopt4(IAst* lpg_TypeArgumentsopt4) { this->lpg_TypeArgumentsopt4 = lpg_TypeArgumentsopt4; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        /**
         * The value returned by <b>getClassBodyopt</b> may be <b>nullptr</b>
         */
        IAst* getClassBodyopt() { return lpg_ClassBodyopt; };
        void setClassBodyopt(IAst* lpg_ClassBodyopt) { this->lpg_ClassBodyopt = lpg_ClassBodyopt; }

        ClassInstanceCreationExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_new,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_ClassOrInterfaceType,
            IAst* lpg_TypeArgumentsopt4,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN,
            IAst* lpg_ClassBodyopt) :Ast(leftIToken, rightIToken) {
            this->lpg_new = lpg_new;
            ((Ast*)lpg_new)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType;
            ((Ast*)lpg_ClassOrInterfaceType)->setParent(this);
            this->lpg_TypeArgumentsopt4 = lpg_TypeArgumentsopt4;
            if (lpg_TypeArgumentsopt4 != nullptr) ((Ast*)lpg_TypeArgumentsopt4)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_ClassBodyopt = lpg_ClassBodyopt;
            if (lpg_ClassBodyopt != nullptr) ((Ast*)lpg_ClassBodyopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_new);
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_ClassOrInterfaceType);
            list.push_back(lpg_TypeArgumentsopt4);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_ClassBodyopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_new->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_ClassOrInterfaceType->accept(v);
                if (lpg_TypeArgumentsopt4 != nullptr) lpg_TypeArgumentsopt4->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
                if (lpg_ClassBodyopt != nullptr) lpg_ClassBodyopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 355:  ClassInstanceCreationExpression ::= Primary . new TypeArgumentsopt identifier TypeArgumentsopt ( ArgumentListopt ) ClassBodyopt
     *</b>
     */
    struct ClassInstanceCreationExpression1 :public Ast
    {
        IAst* lpg_Primary;
        IAst* lpg_DOT;
        IAst* lpg_new;
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_identifier;
        IAst* lpg_TypeArgumentsopt6;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;
        IAst* lpg_ClassBodyopt;

        IAst* getPrimary() { return lpg_Primary; };
        void setPrimary(IAst* lpg_Primary) { this->lpg_Primary = lpg_Primary; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getnew() { return lpg_new; };
        void setnew(IAst* lpg_new) { this->lpg_new = lpg_new; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        /**
         * The value returned by <b>getTypeArgumentsopt6</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt6() { return lpg_TypeArgumentsopt6; };
        void setTypeArgumentsopt6(IAst* lpg_TypeArgumentsopt6) { this->lpg_TypeArgumentsopt6 = lpg_TypeArgumentsopt6; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        /**
         * The value returned by <b>getClassBodyopt</b> may be <b>nullptr</b>
         */
        IAst* getClassBodyopt() { return lpg_ClassBodyopt; };
        void setClassBodyopt(IAst* lpg_ClassBodyopt) { this->lpg_ClassBodyopt = lpg_ClassBodyopt; }

        ClassInstanceCreationExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Primary,
            IAst* lpg_DOT,
            IAst* lpg_new,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_identifier,
            IAst* lpg_TypeArgumentsopt6,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN,
            IAst* lpg_ClassBodyopt) :Ast(leftIToken, rightIToken) {
            this->lpg_Primary = lpg_Primary;
            ((Ast*)lpg_Primary)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_new = lpg_new;
            ((Ast*)lpg_new)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_TypeArgumentsopt6 = lpg_TypeArgumentsopt6;
            if (lpg_TypeArgumentsopt6 != nullptr) ((Ast*)lpg_TypeArgumentsopt6)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_ClassBodyopt = lpg_ClassBodyopt;
            if (lpg_ClassBodyopt != nullptr) ((Ast*)lpg_ClassBodyopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Primary);
            list.push_back(lpg_DOT);
            list.push_back(lpg_new);
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_identifier);
            list.push_back(lpg_TypeArgumentsopt6);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_ClassBodyopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Primary->accept(v);
                lpg_DOT->accept(v);
                lpg_new->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_identifier->accept(v);
                if (lpg_TypeArgumentsopt6 != nullptr) lpg_TypeArgumentsopt6->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
                if (lpg_ClassBodyopt != nullptr) lpg_ClassBodyopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 358:  ArrayCreationExpression ::= new PrimitiveType DimExprs Dimsopt
     *</b>
     */
    struct ArrayCreationExpression0 :public Ast
    {
        IAst* lpg_new;
        IAst* lpg_PrimitiveType;
        IAst* lpg_DimExprs;
        IAst* lpg_Dimsopt;

        IAst* getnew() { return lpg_new; };
        void setnew(IAst* lpg_new) { this->lpg_new = lpg_new; }
        IAst* getPrimitiveType() { return lpg_PrimitiveType; };
        void setPrimitiveType(IAst* lpg_PrimitiveType) { this->lpg_PrimitiveType = lpg_PrimitiveType; }
        IAst* getDimExprs() { return lpg_DimExprs; };
        void setDimExprs(IAst* lpg_DimExprs) { this->lpg_DimExprs = lpg_DimExprs; }
        /**
         * The value returned by <b>getDimsopt</b> may be <b>nullptr</b>
         */
        IAst* getDimsopt() { return lpg_Dimsopt; };
        void setDimsopt(IAst* lpg_Dimsopt) { this->lpg_Dimsopt = lpg_Dimsopt; }

        ArrayCreationExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_new,
            IAst* lpg_PrimitiveType,
            IAst* lpg_DimExprs,
            IAst* lpg_Dimsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_new = lpg_new;
            ((Ast*)lpg_new)->setParent(this);
            this->lpg_PrimitiveType = lpg_PrimitiveType;
            ((Ast*)lpg_PrimitiveType)->setParent(this);
            this->lpg_DimExprs = lpg_DimExprs;
            ((Ast*)lpg_DimExprs)->setParent(this);
            this->lpg_Dimsopt = lpg_Dimsopt;
            if (lpg_Dimsopt != nullptr) ((Ast*)lpg_Dimsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_new);
            list.push_back(lpg_PrimitiveType);
            list.push_back(lpg_DimExprs);
            list.push_back(lpg_Dimsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_new->accept(v);
                lpg_PrimitiveType->accept(v);
                lpg_DimExprs->accept(v);
                if (lpg_Dimsopt != nullptr) lpg_Dimsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 359:  ArrayCreationExpression ::= new ClassOrInterfaceType DimExprs Dimsopt
     *</b>
     */
    struct ArrayCreationExpression1 :public Ast
    {
        IAst* lpg_new;
        IAst* lpg_ClassOrInterfaceType;
        IAst* lpg_DimExprs;
        IAst* lpg_Dimsopt;

        IAst* getnew() { return lpg_new; };
        void setnew(IAst* lpg_new) { this->lpg_new = lpg_new; }
        IAst* getClassOrInterfaceType() { return lpg_ClassOrInterfaceType; };
        void setClassOrInterfaceType(IAst* lpg_ClassOrInterfaceType) { this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType; }
        IAst* getDimExprs() { return lpg_DimExprs; };
        void setDimExprs(IAst* lpg_DimExprs) { this->lpg_DimExprs = lpg_DimExprs; }
        /**
         * The value returned by <b>getDimsopt</b> may be <b>nullptr</b>
         */
        IAst* getDimsopt() { return lpg_Dimsopt; };
        void setDimsopt(IAst* lpg_Dimsopt) { this->lpg_Dimsopt = lpg_Dimsopt; }

        ArrayCreationExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_new,
            IAst* lpg_ClassOrInterfaceType,
            IAst* lpg_DimExprs,
            IAst* lpg_Dimsopt) :Ast(leftIToken, rightIToken) {
            this->lpg_new = lpg_new;
            ((Ast*)lpg_new)->setParent(this);
            this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType;
            ((Ast*)lpg_ClassOrInterfaceType)->setParent(this);
            this->lpg_DimExprs = lpg_DimExprs;
            ((Ast*)lpg_DimExprs)->setParent(this);
            this->lpg_Dimsopt = lpg_Dimsopt;
            if (lpg_Dimsopt != nullptr) ((Ast*)lpg_Dimsopt)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_new);
            list.push_back(lpg_ClassOrInterfaceType);
            list.push_back(lpg_DimExprs);
            list.push_back(lpg_Dimsopt);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_new->accept(v);
                lpg_ClassOrInterfaceType->accept(v);
                lpg_DimExprs->accept(v);
                if (lpg_Dimsopt != nullptr) lpg_Dimsopt->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 360:  ArrayCreationExpression ::= new PrimitiveType Dims ArrayInitializer
     *</b>
     */
    struct ArrayCreationExpression2 :public Ast
    {
        IAst* lpg_new;
        IAst* lpg_PrimitiveType;
        IAst* lpg_Dims;
        IAst* lpg_ArrayInitializer;

        IAst* getnew() { return lpg_new; };
        void setnew(IAst* lpg_new) { this->lpg_new = lpg_new; }
        IAst* getPrimitiveType() { return lpg_PrimitiveType; };
        void setPrimitiveType(IAst* lpg_PrimitiveType) { this->lpg_PrimitiveType = lpg_PrimitiveType; }
        IAst* getDims() { return lpg_Dims; };
        void setDims(IAst* lpg_Dims) { this->lpg_Dims = lpg_Dims; }
        IAst* getArrayInitializer() { return lpg_ArrayInitializer; };
        void setArrayInitializer(IAst* lpg_ArrayInitializer) { this->lpg_ArrayInitializer = lpg_ArrayInitializer; }

        ArrayCreationExpression2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_new,
            IAst* lpg_PrimitiveType,
            IAst* lpg_Dims,
            IAst* lpg_ArrayInitializer) :Ast(leftIToken, rightIToken) {
            this->lpg_new = lpg_new;
            ((Ast*)lpg_new)->setParent(this);
            this->lpg_PrimitiveType = lpg_PrimitiveType;
            ((Ast*)lpg_PrimitiveType)->setParent(this);
            this->lpg_Dims = lpg_Dims;
            ((Ast*)lpg_Dims)->setParent(this);
            this->lpg_ArrayInitializer = lpg_ArrayInitializer;
            ((Ast*)lpg_ArrayInitializer)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_new);
            list.push_back(lpg_PrimitiveType);
            list.push_back(lpg_Dims);
            list.push_back(lpg_ArrayInitializer);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_new->accept(v);
                lpg_PrimitiveType->accept(v);
                lpg_Dims->accept(v);
                lpg_ArrayInitializer->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 361:  ArrayCreationExpression ::= new ClassOrInterfaceType Dims ArrayInitializer
     *</b>
     */
    struct ArrayCreationExpression3 :public Ast
    {
        IAst* lpg_new;
        IAst* lpg_ClassOrInterfaceType;
        IAst* lpg_Dims;
        IAst* lpg_ArrayInitializer;

        IAst* getnew() { return lpg_new; };
        void setnew(IAst* lpg_new) { this->lpg_new = lpg_new; }
        IAst* getClassOrInterfaceType() { return lpg_ClassOrInterfaceType; };
        void setClassOrInterfaceType(IAst* lpg_ClassOrInterfaceType) { this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType; }
        IAst* getDims() { return lpg_Dims; };
        void setDims(IAst* lpg_Dims) { this->lpg_Dims = lpg_Dims; }
        IAst* getArrayInitializer() { return lpg_ArrayInitializer; };
        void setArrayInitializer(IAst* lpg_ArrayInitializer) { this->lpg_ArrayInitializer = lpg_ArrayInitializer; }

        ArrayCreationExpression3(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_new,
            IAst* lpg_ClassOrInterfaceType,
            IAst* lpg_Dims,
            IAst* lpg_ArrayInitializer) :Ast(leftIToken, rightIToken) {
            this->lpg_new = lpg_new;
            ((Ast*)lpg_new)->setParent(this);
            this->lpg_ClassOrInterfaceType = lpg_ClassOrInterfaceType;
            ((Ast*)lpg_ClassOrInterfaceType)->setParent(this);
            this->lpg_Dims = lpg_Dims;
            ((Ast*)lpg_Dims)->setParent(this);
            this->lpg_ArrayInitializer = lpg_ArrayInitializer;
            ((Ast*)lpg_ArrayInitializer)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_new);
            list.push_back(lpg_ClassOrInterfaceType);
            list.push_back(lpg_Dims);
            list.push_back(lpg_ArrayInitializer);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_new->accept(v);
                lpg_ClassOrInterfaceType->accept(v);
                lpg_Dims->accept(v);
                lpg_ArrayInitializer->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 365:  Dims ::= [ ]
     *</b>
     */
    struct Dims0 :public Ast
    {
        IAst* lpg_LBRACKET;
        IAst* lpg_RBRACKET;

        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        Dims0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LBRACKET,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LBRACKET->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 366:  Dims ::= Dims [ ]
     *</b>
     */
    struct Dims1 :public Ast
    {
        IAst* lpg_Dims;
        IAst* lpg_LBRACKET;
        IAst* lpg_RBRACKET;

        IAst* getDims() { return lpg_Dims; };
        void setDims(IAst* lpg_Dims) { this->lpg_Dims = lpg_Dims; }
        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        Dims1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Dims,
            IAst* lpg_LBRACKET,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_Dims = lpg_Dims;
            ((Ast*)lpg_Dims)->setParent(this);
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Dims);
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Dims->accept(v);
                lpg_LBRACKET->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 367:  FieldAccess ::= Primary . identifier
     *</b>
     */
    struct FieldAccess0 :public Ast
    {
        IAst* lpg_Primary;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getPrimary() { return lpg_Primary; };
        void setPrimary(IAst* lpg_Primary) { this->lpg_Primary = lpg_Primary; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        FieldAccess0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Primary,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_Primary = lpg_Primary;
            ((Ast*)lpg_Primary)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Primary);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Primary->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 368:  FieldAccess ::= super . identifier
     *</b>
     */
    struct FieldAccess1 :public Ast
    {
        IAst* lpg_super;
        IAst* lpg_DOT;
        IAst* lpg_identifier;

        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        FieldAccess1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_super,
            IAst* lpg_DOT,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_super);
            list.push_back(lpg_DOT);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_super->accept(v);
                lpg_DOT->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 369:  FieldAccess ::= ClassName . super . identifier
     *</b>
     */
    struct FieldAccess2 :public Ast
    {
        IAst* lpg_ClassName;
        IAst* lpg_DOT;
        IAst* lpg_super;
        IAst* lpg_DOT4;
        IAst* lpg_identifier;

        IAst* getClassName() { return lpg_ClassName; };
        void setClassName(IAst* lpg_ClassName) { this->lpg_ClassName = lpg_ClassName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getDOT4() { return lpg_DOT4; };
        void setDOT4(IAst* lpg_DOT4) { this->lpg_DOT4 = lpg_DOT4; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }

        FieldAccess2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassName,
            IAst* lpg_DOT,
            IAst* lpg_super,
            IAst* lpg_DOT4,
            IAst* lpg_identifier) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassName = lpg_ClassName;
            ((Ast*)lpg_ClassName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_DOT4 = lpg_DOT4;
            ((Ast*)lpg_DOT4)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_super);
            list.push_back(lpg_DOT4);
            list.push_back(lpg_identifier);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ClassName->accept(v);
                lpg_DOT->accept(v);
                lpg_super->accept(v);
                lpg_DOT4->accept(v);
                lpg_identifier->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 370:  MethodInvocation ::= MethodName ( ArgumentListopt )
     *</b>
     */
    struct MethodInvocation0 :public Ast
    {
        IAst* lpg_MethodName;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;

        IAst* getMethodName() { return lpg_MethodName; };
        void setMethodName(IAst* lpg_MethodName) { this->lpg_MethodName = lpg_MethodName; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        MethodInvocation0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MethodName,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_MethodName = lpg_MethodName;
            ((Ast*)lpg_MethodName)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MethodName);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MethodName->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 371:  MethodInvocation ::= Primary . TypeArgumentsopt identifier ( ArgumentListopt )
     *</b>
     */
    struct MethodInvocation1 :public Ast
    {
        IAst* lpg_Primary;
        IAst* lpg_DOT;
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_identifier;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;

        IAst* getPrimary() { return lpg_Primary; };
        void setPrimary(IAst* lpg_Primary) { this->lpg_Primary = lpg_Primary; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        MethodInvocation1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_Primary,
            IAst* lpg_DOT,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_identifier,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_Primary = lpg_Primary;
            ((Ast*)lpg_Primary)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_Primary);
            list.push_back(lpg_DOT);
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_identifier);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_Primary->accept(v);
                lpg_DOT->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_identifier->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 372:  MethodInvocation ::= super . TypeArgumentsopt identifier ( ArgumentListopt )
     *</b>
     */
    struct MethodInvocation2 :public Ast
    {
        IAst* lpg_super;
        IAst* lpg_DOT;
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_identifier;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;

        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        MethodInvocation2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_super,
            IAst* lpg_DOT,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_identifier,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_super);
            list.push_back(lpg_DOT);
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_identifier);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_super->accept(v);
                lpg_DOT->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_identifier->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 373:  MethodInvocation ::= ClassName . super . TypeArgumentsopt identifier ( ArgumentListopt )
     *</b>
     */
    struct MethodInvocation3 :public Ast
    {
        IAst* lpg_ClassName;
        IAst* lpg_DOT;
        IAst* lpg_super;
        IAst* lpg_DOT4;
        IAst* lpg_TypeArgumentsopt;
        IAst* lpg_identifier;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;

        IAst* getClassName() { return lpg_ClassName; };
        void setClassName(IAst* lpg_ClassName) { this->lpg_ClassName = lpg_ClassName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getsuper() { return lpg_super; };
        void setsuper(IAst* lpg_super) { this->lpg_super = lpg_super; }
        IAst* getDOT4() { return lpg_DOT4; };
        void setDOT4(IAst* lpg_DOT4) { this->lpg_DOT4 = lpg_DOT4; }
        /**
         * The value returned by <b>getTypeArgumentsopt</b> may be <b>nullptr</b>
         */
        IAst* getTypeArgumentsopt() { return lpg_TypeArgumentsopt; };
        void setTypeArgumentsopt(IAst* lpg_TypeArgumentsopt) { this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        MethodInvocation3(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ClassName,
            IAst* lpg_DOT,
            IAst* lpg_super,
            IAst* lpg_DOT4,
            IAst* lpg_TypeArgumentsopt,
            IAst* lpg_identifier,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_ClassName = lpg_ClassName;
            ((Ast*)lpg_ClassName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_super = lpg_super;
            ((Ast*)lpg_super)->setParent(this);
            this->lpg_DOT4 = lpg_DOT4;
            ((Ast*)lpg_DOT4)->setParent(this);
            this->lpg_TypeArgumentsopt = lpg_TypeArgumentsopt;
            if (lpg_TypeArgumentsopt != nullptr) ((Ast*)lpg_TypeArgumentsopt)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ClassName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_super);
            list.push_back(lpg_DOT4);
            list.push_back(lpg_TypeArgumentsopt);
            list.push_back(lpg_identifier);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ClassName->accept(v);
                lpg_DOT->accept(v);
                lpg_super->accept(v);
                lpg_DOT4->accept(v);
                if (lpg_TypeArgumentsopt != nullptr) lpg_TypeArgumentsopt->accept(v);
                lpg_identifier->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 374:  MethodInvocation ::= TypeName . TypeArguments identifier ( ArgumentListopt )
     *</b>
     */
    struct MethodInvocation4 :public Ast
    {
        IAst* lpg_TypeName;
        IAst* lpg_DOT;
        IAst* lpg_TypeArguments;
        IAst* lpg_identifier;
        IAst* lpg_LPAREN;
        IAst* lpg_ArgumentListopt;
        IAst* lpg_RPAREN;

        IAst* getTypeName() { return lpg_TypeName; };
        void setTypeName(IAst* lpg_TypeName) { this->lpg_TypeName = lpg_TypeName; }
        IAst* getDOT() { return lpg_DOT; };
        void setDOT(IAst* lpg_DOT) { this->lpg_DOT = lpg_DOT; }
        IAst* getTypeArguments() { return lpg_TypeArguments; };
        void setTypeArguments(IAst* lpg_TypeArguments) { this->lpg_TypeArguments = lpg_TypeArguments; }
        IAst* getidentifier() { return lpg_identifier; };
        void setidentifier(IAst* lpg_identifier) { this->lpg_identifier = lpg_identifier; }
        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        /**
         * The value returned by <b>getArgumentListopt</b> may be <b>nullptr</b>
         */
        IAst* getArgumentListopt() { return lpg_ArgumentListopt; };
        void setArgumentListopt(IAst* lpg_ArgumentListopt) { this->lpg_ArgumentListopt = lpg_ArgumentListopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }

        MethodInvocation4(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TypeName,
            IAst* lpg_DOT,
            IAst* lpg_TypeArguments,
            IAst* lpg_identifier,
            IAst* lpg_LPAREN,
            IAst* lpg_ArgumentListopt,
            IAst* lpg_RPAREN) :Ast(leftIToken, rightIToken) {
            this->lpg_TypeName = lpg_TypeName;
            ((Ast*)lpg_TypeName)->setParent(this);
            this->lpg_DOT = lpg_DOT;
            ((Ast*)lpg_DOT)->setParent(this);
            this->lpg_TypeArguments = lpg_TypeArguments;
            ((Ast*)lpg_TypeArguments)->setParent(this);
            this->lpg_identifier = lpg_identifier;
            ((Ast*)lpg_identifier)->setParent(this);
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ArgumentListopt = lpg_ArgumentListopt;
            if (lpg_ArgumentListopt != nullptr) ((Ast*)lpg_ArgumentListopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TypeName);
            list.push_back(lpg_DOT);
            list.push_back(lpg_TypeArguments);
            list.push_back(lpg_identifier);
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ArgumentListopt);
            list.push_back(lpg_RPAREN);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TypeName->accept(v);
                lpg_DOT->accept(v);
                lpg_TypeArguments->accept(v);
                lpg_identifier->accept(v);
                lpg_LPAREN->accept(v);
                if (lpg_ArgumentListopt != nullptr) lpg_ArgumentListopt->accept(v);
                lpg_RPAREN->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 375:  ArrayAccess ::= ExpressionName [ Expression ]
     *</b>
     */
    struct ArrayAccess0 :public Ast
    {
        IAst* lpg_ExpressionName;
        IAst* lpg_LBRACKET;
        IAst* lpg_Expression;
        IAst* lpg_RBRACKET;

        IAst* getExpressionName() { return lpg_ExpressionName; };
        void setExpressionName(IAst* lpg_ExpressionName) { this->lpg_ExpressionName = lpg_ExpressionName; }
        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        ArrayAccess0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ExpressionName,
            IAst* lpg_LBRACKET,
            IAst* lpg_Expression,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_ExpressionName = lpg_ExpressionName;
            ((Ast*)lpg_ExpressionName)->setParent(this);
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ExpressionName);
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ExpressionName->accept(v);
                lpg_LBRACKET->accept(v);
                lpg_Expression->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 376:  ArrayAccess ::= PrimaryNoNewArray [ Expression ]
     *</b>
     */
    struct ArrayAccess1 :public Ast
    {
        IAst* lpg_PrimaryNoNewArray;
        IAst* lpg_LBRACKET;
        IAst* lpg_Expression;
        IAst* lpg_RBRACKET;

        IAst* getPrimaryNoNewArray() { return lpg_PrimaryNoNewArray; };
        void setPrimaryNoNewArray(IAst* lpg_PrimaryNoNewArray) { this->lpg_PrimaryNoNewArray = lpg_PrimaryNoNewArray; }
        IAst* getLBRACKET() { return lpg_LBRACKET; };
        void setLBRACKET(IAst* lpg_LBRACKET) { this->lpg_LBRACKET = lpg_LBRACKET; }
        IAst* getExpression() { return lpg_Expression; };
        void setExpression(IAst* lpg_Expression) { this->lpg_Expression = lpg_Expression; }
        IAst* getRBRACKET() { return lpg_RBRACKET; };
        void setRBRACKET(IAst* lpg_RBRACKET) { this->lpg_RBRACKET = lpg_RBRACKET; }

        ArrayAccess1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PrimaryNoNewArray,
            IAst* lpg_LBRACKET,
            IAst* lpg_Expression,
            IAst* lpg_RBRACKET) :Ast(leftIToken, rightIToken) {
            this->lpg_PrimaryNoNewArray = lpg_PrimaryNoNewArray;
            ((Ast*)lpg_PrimaryNoNewArray)->setParent(this);
            this->lpg_LBRACKET = lpg_LBRACKET;
            ((Ast*)lpg_LBRACKET)->setParent(this);
            this->lpg_Expression = lpg_Expression;
            ((Ast*)lpg_Expression)->setParent(this);
            this->lpg_RBRACKET = lpg_RBRACKET;
            ((Ast*)lpg_RBRACKET)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PrimaryNoNewArray);
            list.push_back(lpg_LBRACKET);
            list.push_back(lpg_Expression);
            list.push_back(lpg_RBRACKET);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PrimaryNoNewArray->accept(v);
                lpg_LBRACKET->accept(v);
                lpg_Expression->accept(v);
                lpg_RBRACKET->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 385:  UnaryExpression ::= + UnaryExpression
     *</b>
     */
    struct UnaryExpression0 :public Ast
    {
        IAst* lpg_PLUS;
        IAst* lpg_UnaryExpression;

        IAst* getPLUS() { return lpg_PLUS; };
        void setPLUS(IAst* lpg_PLUS) { this->lpg_PLUS = lpg_PLUS; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        UnaryExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_PLUS,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_PLUS = lpg_PLUS;
            ((Ast*)lpg_PLUS)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_PLUS);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_PLUS->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 386:  UnaryExpression ::= - UnaryExpression
     *</b>
     */
    struct UnaryExpression1 :public Ast
    {
        IAst* lpg_MINUS;
        IAst* lpg_UnaryExpression;

        IAst* getMINUS() { return lpg_MINUS; };
        void setMINUS(IAst* lpg_MINUS) { this->lpg_MINUS = lpg_MINUS; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        UnaryExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MINUS,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_MINUS = lpg_MINUS;
            ((Ast*)lpg_MINUS)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MINUS);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MINUS->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 391:  UnaryExpressionNotPlusMinus ::= ~ UnaryExpression
     *</b>
     */
    struct UnaryExpressionNotPlusMinus0 :public Ast
    {
        IAst* lpg_TWIDDLE;
        IAst* lpg_UnaryExpression;

        IAst* getTWIDDLE() { return lpg_TWIDDLE; };
        void setTWIDDLE(IAst* lpg_TWIDDLE) { this->lpg_TWIDDLE = lpg_TWIDDLE; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        UnaryExpressionNotPlusMinus0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_TWIDDLE,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_TWIDDLE = lpg_TWIDDLE;
            ((Ast*)lpg_TWIDDLE)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_TWIDDLE);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_TWIDDLE->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 392:  UnaryExpressionNotPlusMinus ::= ! UnaryExpression
     *</b>
     */
    struct UnaryExpressionNotPlusMinus1 :public Ast
    {
        IAst* lpg_NOT;
        IAst* lpg_UnaryExpression;

        IAst* getNOT() { return lpg_NOT; };
        void setNOT(IAst* lpg_NOT) { this->lpg_NOT = lpg_NOT; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        UnaryExpressionNotPlusMinus1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_NOT,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_NOT = lpg_NOT;
            ((Ast*)lpg_NOT)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_NOT);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_NOT->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 394:  CastExpression ::= ( PrimitiveType Dimsopt ) UnaryExpression
     *</b>
     */
    struct CastExpression0 :public Ast
    {
        IAst* lpg_LPAREN;
        IAst* lpg_PrimitiveType;
        IAst* lpg_Dimsopt;
        IAst* lpg_RPAREN;
        IAst* lpg_UnaryExpression;

        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getPrimitiveType() { return lpg_PrimitiveType; };
        void setPrimitiveType(IAst* lpg_PrimitiveType) { this->lpg_PrimitiveType = lpg_PrimitiveType; }
        /**
         * The value returned by <b>getDimsopt</b> may be <b>nullptr</b>
         */
        IAst* getDimsopt() { return lpg_Dimsopt; };
        void setDimsopt(IAst* lpg_Dimsopt) { this->lpg_Dimsopt = lpg_Dimsopt; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        CastExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LPAREN,
            IAst* lpg_PrimitiveType,
            IAst* lpg_Dimsopt,
            IAst* lpg_RPAREN,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_PrimitiveType = lpg_PrimitiveType;
            ((Ast*)lpg_PrimitiveType)->setParent(this);
            this->lpg_Dimsopt = lpg_Dimsopt;
            if (lpg_Dimsopt != nullptr) ((Ast*)lpg_Dimsopt)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_PrimitiveType);
            list.push_back(lpg_Dimsopt);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LPAREN->accept(v);
                lpg_PrimitiveType->accept(v);
                if (lpg_Dimsopt != nullptr) lpg_Dimsopt->accept(v);
                lpg_RPAREN->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 395:  CastExpression ::= ( ReferenceType ) UnaryExpressionNotPlusMinus
     *</b>
     */
    struct CastExpression1 :public Ast
    {
        IAst* lpg_LPAREN;
        IAst* lpg_ReferenceType;
        IAst* lpg_RPAREN;
        IAst* lpg_UnaryExpressionNotPlusMinus;

        IAst* getLPAREN() { return lpg_LPAREN; };
        void setLPAREN(IAst* lpg_LPAREN) { this->lpg_LPAREN = lpg_LPAREN; }
        IAst* getReferenceType() { return lpg_ReferenceType; };
        void setReferenceType(IAst* lpg_ReferenceType) { this->lpg_ReferenceType = lpg_ReferenceType; }
        IAst* getRPAREN() { return lpg_RPAREN; };
        void setRPAREN(IAst* lpg_RPAREN) { this->lpg_RPAREN = lpg_RPAREN; }
        IAst* getUnaryExpressionNotPlusMinus() { return lpg_UnaryExpressionNotPlusMinus; };
        void setUnaryExpressionNotPlusMinus(IAst* lpg_UnaryExpressionNotPlusMinus) { this->lpg_UnaryExpressionNotPlusMinus = lpg_UnaryExpressionNotPlusMinus; }

        CastExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_LPAREN,
            IAst* lpg_ReferenceType,
            IAst* lpg_RPAREN,
            IAst* lpg_UnaryExpressionNotPlusMinus) :Ast(leftIToken, rightIToken) {
            this->lpg_LPAREN = lpg_LPAREN;
            ((Ast*)lpg_LPAREN)->setParent(this);
            this->lpg_ReferenceType = lpg_ReferenceType;
            ((Ast*)lpg_ReferenceType)->setParent(this);
            this->lpg_RPAREN = lpg_RPAREN;
            ((Ast*)lpg_RPAREN)->setParent(this);
            this->lpg_UnaryExpressionNotPlusMinus = lpg_UnaryExpressionNotPlusMinus;
            ((Ast*)lpg_UnaryExpressionNotPlusMinus)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_LPAREN);
            list.push_back(lpg_ReferenceType);
            list.push_back(lpg_RPAREN);
            list.push_back(lpg_UnaryExpressionNotPlusMinus);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_LPAREN->accept(v);
                lpg_ReferenceType->accept(v);
                lpg_RPAREN->accept(v);
                lpg_UnaryExpressionNotPlusMinus->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 397:  MultiplicativeExpression ::= MultiplicativeExpression * UnaryExpression
     *</b>
     */
    struct MultiplicativeExpression0 :public Ast
    {
        IAst* lpg_MultiplicativeExpression;
        IAst* lpg_MULTIPLY;
        IAst* lpg_UnaryExpression;

        IAst* getMultiplicativeExpression() { return lpg_MultiplicativeExpression; };
        void setMultiplicativeExpression(IAst* lpg_MultiplicativeExpression) { this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression; }
        IAst* getMULTIPLY() { return lpg_MULTIPLY; };
        void setMULTIPLY(IAst* lpg_MULTIPLY) { this->lpg_MULTIPLY = lpg_MULTIPLY; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        MultiplicativeExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MultiplicativeExpression,
            IAst* lpg_MULTIPLY,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression;
            ((Ast*)lpg_MultiplicativeExpression)->setParent(this);
            this->lpg_MULTIPLY = lpg_MULTIPLY;
            ((Ast*)lpg_MULTIPLY)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MultiplicativeExpression);
            list.push_back(lpg_MULTIPLY);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MultiplicativeExpression->accept(v);
                lpg_MULTIPLY->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 398:  MultiplicativeExpression ::= MultiplicativeExpression / UnaryExpression
     *</b>
     */
    struct MultiplicativeExpression1 :public Ast
    {
        IAst* lpg_MultiplicativeExpression;
        IAst* lpg_DIVIDE;
        IAst* lpg_UnaryExpression;

        IAst* getMultiplicativeExpression() { return lpg_MultiplicativeExpression; };
        void setMultiplicativeExpression(IAst* lpg_MultiplicativeExpression) { this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression; }
        IAst* getDIVIDE() { return lpg_DIVIDE; };
        void setDIVIDE(IAst* lpg_DIVIDE) { this->lpg_DIVIDE = lpg_DIVIDE; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        MultiplicativeExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MultiplicativeExpression,
            IAst* lpg_DIVIDE,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression;
            ((Ast*)lpg_MultiplicativeExpression)->setParent(this);
            this->lpg_DIVIDE = lpg_DIVIDE;
            ((Ast*)lpg_DIVIDE)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MultiplicativeExpression);
            list.push_back(lpg_DIVIDE);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MultiplicativeExpression->accept(v);
                lpg_DIVIDE->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 399:  MultiplicativeExpression ::= MultiplicativeExpression % UnaryExpression
     *</b>
     */
    struct MultiplicativeExpression2 :public Ast
    {
        IAst* lpg_MultiplicativeExpression;
        IAst* lpg_REMAINDER;
        IAst* lpg_UnaryExpression;

        IAst* getMultiplicativeExpression() { return lpg_MultiplicativeExpression; };
        void setMultiplicativeExpression(IAst* lpg_MultiplicativeExpression) { this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression; }
        IAst* getREMAINDER() { return lpg_REMAINDER; };
        void setREMAINDER(IAst* lpg_REMAINDER) { this->lpg_REMAINDER = lpg_REMAINDER; }
        IAst* getUnaryExpression() { return lpg_UnaryExpression; };
        void setUnaryExpression(IAst* lpg_UnaryExpression) { this->lpg_UnaryExpression = lpg_UnaryExpression; }

        MultiplicativeExpression2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_MultiplicativeExpression,
            IAst* lpg_REMAINDER,
            IAst* lpg_UnaryExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression;
            ((Ast*)lpg_MultiplicativeExpression)->setParent(this);
            this->lpg_REMAINDER = lpg_REMAINDER;
            ((Ast*)lpg_REMAINDER)->setParent(this);
            this->lpg_UnaryExpression = lpg_UnaryExpression;
            ((Ast*)lpg_UnaryExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_MultiplicativeExpression);
            list.push_back(lpg_REMAINDER);
            list.push_back(lpg_UnaryExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_MultiplicativeExpression->accept(v);
                lpg_REMAINDER->accept(v);
                lpg_UnaryExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 401:  AdditiveExpression ::= AdditiveExpression + MultiplicativeExpression
     *</b>
     */
    struct AdditiveExpression0 :public Ast
    {
        IAst* lpg_AdditiveExpression;
        IAst* lpg_PLUS;
        IAst* lpg_MultiplicativeExpression;

        IAst* getAdditiveExpression() { return lpg_AdditiveExpression; };
        void setAdditiveExpression(IAst* lpg_AdditiveExpression) { this->lpg_AdditiveExpression = lpg_AdditiveExpression; }
        IAst* getPLUS() { return lpg_PLUS; };
        void setPLUS(IAst* lpg_PLUS) { this->lpg_PLUS = lpg_PLUS; }
        IAst* getMultiplicativeExpression() { return lpg_MultiplicativeExpression; };
        void setMultiplicativeExpression(IAst* lpg_MultiplicativeExpression) { this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression; }

        AdditiveExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AdditiveExpression,
            IAst* lpg_PLUS,
            IAst* lpg_MultiplicativeExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_AdditiveExpression = lpg_AdditiveExpression;
            ((Ast*)lpg_AdditiveExpression)->setParent(this);
            this->lpg_PLUS = lpg_PLUS;
            ((Ast*)lpg_PLUS)->setParent(this);
            this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression;
            ((Ast*)lpg_MultiplicativeExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AdditiveExpression);
            list.push_back(lpg_PLUS);
            list.push_back(lpg_MultiplicativeExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AdditiveExpression->accept(v);
                lpg_PLUS->accept(v);
                lpg_MultiplicativeExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 402:  AdditiveExpression ::= AdditiveExpression - MultiplicativeExpression
     *</b>
     */
    struct AdditiveExpression1 :public Ast
    {
        IAst* lpg_AdditiveExpression;
        IAst* lpg_MINUS;
        IAst* lpg_MultiplicativeExpression;

        IAst* getAdditiveExpression() { return lpg_AdditiveExpression; };
        void setAdditiveExpression(IAst* lpg_AdditiveExpression) { this->lpg_AdditiveExpression = lpg_AdditiveExpression; }
        IAst* getMINUS() { return lpg_MINUS; };
        void setMINUS(IAst* lpg_MINUS) { this->lpg_MINUS = lpg_MINUS; }
        IAst* getMultiplicativeExpression() { return lpg_MultiplicativeExpression; };
        void setMultiplicativeExpression(IAst* lpg_MultiplicativeExpression) { this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression; }

        AdditiveExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_AdditiveExpression,
            IAst* lpg_MINUS,
            IAst* lpg_MultiplicativeExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_AdditiveExpression = lpg_AdditiveExpression;
            ((Ast*)lpg_AdditiveExpression)->setParent(this);
            this->lpg_MINUS = lpg_MINUS;
            ((Ast*)lpg_MINUS)->setParent(this);
            this->lpg_MultiplicativeExpression = lpg_MultiplicativeExpression;
            ((Ast*)lpg_MultiplicativeExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_AdditiveExpression);
            list.push_back(lpg_MINUS);
            list.push_back(lpg_MultiplicativeExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_AdditiveExpression->accept(v);
                lpg_MINUS->accept(v);
                lpg_MultiplicativeExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 404:  ShiftExpression ::= ShiftExpression << AdditiveExpression
     *</b>
     */
    struct ShiftExpression0 :public Ast
    {
        IAst* lpg_ShiftExpression;
        IAst* lpg_LEFT_SHIFT;
        IAst* lpg_AdditiveExpression;

        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }
        IAst* getLEFT_SHIFT() { return lpg_LEFT_SHIFT; };
        void setLEFT_SHIFT(IAst* lpg_LEFT_SHIFT) { this->lpg_LEFT_SHIFT = lpg_LEFT_SHIFT; }
        IAst* getAdditiveExpression() { return lpg_AdditiveExpression; };
        void setAdditiveExpression(IAst* lpg_AdditiveExpression) { this->lpg_AdditiveExpression = lpg_AdditiveExpression; }

        ShiftExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ShiftExpression,
            IAst* lpg_LEFT_SHIFT,
            IAst* lpg_AdditiveExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            this->lpg_LEFT_SHIFT = lpg_LEFT_SHIFT;
            ((Ast*)lpg_LEFT_SHIFT)->setParent(this);
            this->lpg_AdditiveExpression = lpg_AdditiveExpression;
            ((Ast*)lpg_AdditiveExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ShiftExpression);
            list.push_back(lpg_LEFT_SHIFT);
            list.push_back(lpg_AdditiveExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ShiftExpression->accept(v);
                lpg_LEFT_SHIFT->accept(v);
                lpg_AdditiveExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 405:  ShiftExpression ::= ShiftExpression > > AdditiveExpression
     *</b>
     */
    struct ShiftExpression1 :public Ast
    {
        IAst* lpg_ShiftExpression;
        IAst* lpg_GREATER;
        IAst* lpg_GREATER3;
        IAst* lpg_AdditiveExpression;

        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }
        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }
        IAst* getGREATER3() { return lpg_GREATER3; };
        void setGREATER3(IAst* lpg_GREATER3) { this->lpg_GREATER3 = lpg_GREATER3; }
        IAst* getAdditiveExpression() { return lpg_AdditiveExpression; };
        void setAdditiveExpression(IAst* lpg_AdditiveExpression) { this->lpg_AdditiveExpression = lpg_AdditiveExpression; }

        ShiftExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ShiftExpression,
            IAst* lpg_GREATER,
            IAst* lpg_GREATER3,
            IAst* lpg_AdditiveExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            this->lpg_GREATER3 = lpg_GREATER3;
            ((Ast*)lpg_GREATER3)->setParent(this);
            this->lpg_AdditiveExpression = lpg_AdditiveExpression;
            ((Ast*)lpg_AdditiveExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ShiftExpression);
            list.push_back(lpg_GREATER);
            list.push_back(lpg_GREATER3);
            list.push_back(lpg_AdditiveExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ShiftExpression->accept(v);
                lpg_GREATER->accept(v);
                lpg_GREATER3->accept(v);
                lpg_AdditiveExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 406:  ShiftExpression ::= ShiftExpression > > > AdditiveExpression
     *</b>
     */
    struct ShiftExpression2 :public Ast
    {
        IAst* lpg_ShiftExpression;
        IAst* lpg_GREATER;
        IAst* lpg_GREATER3;
        IAst* lpg_GREATER4;
        IAst* lpg_AdditiveExpression;

        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }
        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }
        IAst* getGREATER3() { return lpg_GREATER3; };
        void setGREATER3(IAst* lpg_GREATER3) { this->lpg_GREATER3 = lpg_GREATER3; }
        IAst* getGREATER4() { return lpg_GREATER4; };
        void setGREATER4(IAst* lpg_GREATER4) { this->lpg_GREATER4 = lpg_GREATER4; }
        IAst* getAdditiveExpression() { return lpg_AdditiveExpression; };
        void setAdditiveExpression(IAst* lpg_AdditiveExpression) { this->lpg_AdditiveExpression = lpg_AdditiveExpression; }

        ShiftExpression2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_ShiftExpression,
            IAst* lpg_GREATER,
            IAst* lpg_GREATER3,
            IAst* lpg_GREATER4,
            IAst* lpg_AdditiveExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            this->lpg_GREATER3 = lpg_GREATER3;
            ((Ast*)lpg_GREATER3)->setParent(this);
            this->lpg_GREATER4 = lpg_GREATER4;
            ((Ast*)lpg_GREATER4)->setParent(this);
            this->lpg_AdditiveExpression = lpg_AdditiveExpression;
            ((Ast*)lpg_AdditiveExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_ShiftExpression);
            list.push_back(lpg_GREATER);
            list.push_back(lpg_GREATER3);
            list.push_back(lpg_GREATER4);
            list.push_back(lpg_AdditiveExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_ShiftExpression->accept(v);
                lpg_GREATER->accept(v);
                lpg_GREATER3->accept(v);
                lpg_GREATER4->accept(v);
                lpg_AdditiveExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 408:  RelationalExpression ::= RelationalExpression < ShiftExpression
     *</b>
     */
    struct RelationalExpression0 :public Ast
    {
        IAst* lpg_RelationalExpression;
        IAst* lpg_LESS;
        IAst* lpg_ShiftExpression;

        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }
        IAst* getLESS() { return lpg_LESS; };
        void setLESS(IAst* lpg_LESS) { this->lpg_LESS = lpg_LESS; }
        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }

        RelationalExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_RelationalExpression,
            IAst* lpg_LESS,
            IAst* lpg_ShiftExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            this->lpg_LESS = lpg_LESS;
            ((Ast*)lpg_LESS)->setParent(this);
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_RelationalExpression);
            list.push_back(lpg_LESS);
            list.push_back(lpg_ShiftExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_RelationalExpression->accept(v);
                lpg_LESS->accept(v);
                lpg_ShiftExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 409:  RelationalExpression ::= RelationalExpression > ShiftExpression
     *</b>
     */
    struct RelationalExpression1 :public Ast
    {
        IAst* lpg_RelationalExpression;
        IAst* lpg_GREATER;
        IAst* lpg_ShiftExpression;

        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }
        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }
        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }

        RelationalExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_RelationalExpression,
            IAst* lpg_GREATER,
            IAst* lpg_ShiftExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_RelationalExpression);
            list.push_back(lpg_GREATER);
            list.push_back(lpg_ShiftExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_RelationalExpression->accept(v);
                lpg_GREATER->accept(v);
                lpg_ShiftExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 410:  RelationalExpression ::= RelationalExpression <= ShiftExpression
     *</b>
     */
    struct RelationalExpression2 :public Ast
    {
        IAst* lpg_RelationalExpression;
        IAst* lpg_LESS_EQUAL;
        IAst* lpg_ShiftExpression;

        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }
        IAst* getLESS_EQUAL() { return lpg_LESS_EQUAL; };
        void setLESS_EQUAL(IAst* lpg_LESS_EQUAL) { this->lpg_LESS_EQUAL = lpg_LESS_EQUAL; }
        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }

        RelationalExpression2(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_RelationalExpression,
            IAst* lpg_LESS_EQUAL,
            IAst* lpg_ShiftExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            this->lpg_LESS_EQUAL = lpg_LESS_EQUAL;
            ((Ast*)lpg_LESS_EQUAL)->setParent(this);
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_RelationalExpression);
            list.push_back(lpg_LESS_EQUAL);
            list.push_back(lpg_ShiftExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_RelationalExpression->accept(v);
                lpg_LESS_EQUAL->accept(v);
                lpg_ShiftExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 411:  RelationalExpression ::= RelationalExpression > = ShiftExpression
     *</b>
     */
    struct RelationalExpression3 :public Ast
    {
        IAst* lpg_RelationalExpression;
        IAst* lpg_GREATER;
        IAst* lpg_EQUAL;
        IAst* lpg_ShiftExpression;

        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }
        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }
        IAst* getEQUAL() { return lpg_EQUAL; };
        void setEQUAL(IAst* lpg_EQUAL) { this->lpg_EQUAL = lpg_EQUAL; }
        IAst* getShiftExpression() { return lpg_ShiftExpression; };
        void setShiftExpression(IAst* lpg_ShiftExpression) { this->lpg_ShiftExpression = lpg_ShiftExpression; }

        RelationalExpression3(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_RelationalExpression,
            IAst* lpg_GREATER,
            IAst* lpg_EQUAL,
            IAst* lpg_ShiftExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            this->lpg_EQUAL = lpg_EQUAL;
            ((Ast*)lpg_EQUAL)->setParent(this);
            this->lpg_ShiftExpression = lpg_ShiftExpression;
            ((Ast*)lpg_ShiftExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_RelationalExpression);
            list.push_back(lpg_GREATER);
            list.push_back(lpg_EQUAL);
            list.push_back(lpg_ShiftExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_RelationalExpression->accept(v);
                lpg_GREATER->accept(v);
                lpg_EQUAL->accept(v);
                lpg_ShiftExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 412:  RelationalExpression ::= RelationalExpression instanceof ReferenceType
     *</b>
     */
    struct RelationalExpression4 :public Ast
    {
        IAst* lpg_RelationalExpression;
        IAst* lpg_instanceof;
        IAst* lpg_ReferenceType;

        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }
        IAst* getinstanceof() { return lpg_instanceof; };
        void setinstanceof(IAst* lpg_instanceof) { this->lpg_instanceof = lpg_instanceof; }
        IAst* getReferenceType() { return lpg_ReferenceType; };
        void setReferenceType(IAst* lpg_ReferenceType) { this->lpg_ReferenceType = lpg_ReferenceType; }

        RelationalExpression4(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_RelationalExpression,
            IAst* lpg_instanceof,
            IAst* lpg_ReferenceType) :Ast(leftIToken, rightIToken) {
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            this->lpg_instanceof = lpg_instanceof;
            ((Ast*)lpg_instanceof)->setParent(this);
            this->lpg_ReferenceType = lpg_ReferenceType;
            ((Ast*)lpg_ReferenceType)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_RelationalExpression);
            list.push_back(lpg_instanceof);
            list.push_back(lpg_ReferenceType);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_RelationalExpression->accept(v);
                lpg_instanceof->accept(v);
                lpg_ReferenceType->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 414:  EqualityExpression ::= EqualityExpression == RelationalExpression
     *</b>
     */
    struct EqualityExpression0 :public Ast
    {
        IAst* lpg_EqualityExpression;
        IAst* lpg_EQUAL_EQUAL;
        IAst* lpg_RelationalExpression;

        IAst* getEqualityExpression() { return lpg_EqualityExpression; };
        void setEqualityExpression(IAst* lpg_EqualityExpression) { this->lpg_EqualityExpression = lpg_EqualityExpression; }
        IAst* getEQUAL_EQUAL() { return lpg_EQUAL_EQUAL; };
        void setEQUAL_EQUAL(IAst* lpg_EQUAL_EQUAL) { this->lpg_EQUAL_EQUAL = lpg_EQUAL_EQUAL; }
        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }

        EqualityExpression0(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_EqualityExpression,
            IAst* lpg_EQUAL_EQUAL,
            IAst* lpg_RelationalExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_EqualityExpression = lpg_EqualityExpression;
            ((Ast*)lpg_EqualityExpression)->setParent(this);
            this->lpg_EQUAL_EQUAL = lpg_EQUAL_EQUAL;
            ((Ast*)lpg_EQUAL_EQUAL)->setParent(this);
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_EqualityExpression);
            list.push_back(lpg_EQUAL_EQUAL);
            list.push_back(lpg_RelationalExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_EqualityExpression->accept(v);
                lpg_EQUAL_EQUAL->accept(v);
                lpg_RelationalExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 415:  EqualityExpression ::= EqualityExpression != RelationalExpression
     *</b>
     */
    struct EqualityExpression1 :public Ast
    {
        IAst* lpg_EqualityExpression;
        IAst* lpg_NOT_EQUAL;
        IAst* lpg_RelationalExpression;

        IAst* getEqualityExpression() { return lpg_EqualityExpression; };
        void setEqualityExpression(IAst* lpg_EqualityExpression) { this->lpg_EqualityExpression = lpg_EqualityExpression; }
        IAst* getNOT_EQUAL() { return lpg_NOT_EQUAL; };
        void setNOT_EQUAL(IAst* lpg_NOT_EQUAL) { this->lpg_NOT_EQUAL = lpg_NOT_EQUAL; }
        IAst* getRelationalExpression() { return lpg_RelationalExpression; };
        void setRelationalExpression(IAst* lpg_RelationalExpression) { this->lpg_RelationalExpression = lpg_RelationalExpression; }

        EqualityExpression1(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_EqualityExpression,
            IAst* lpg_NOT_EQUAL,
            IAst* lpg_RelationalExpression) :Ast(leftIToken, rightIToken) {
            this->lpg_EqualityExpression = lpg_EqualityExpression;
            ((Ast*)lpg_EqualityExpression)->setParent(this);
            this->lpg_NOT_EQUAL = lpg_NOT_EQUAL;
            ((Ast*)lpg_NOT_EQUAL)->setParent(this);
            this->lpg_RelationalExpression = lpg_RelationalExpression;
            ((Ast*)lpg_RelationalExpression)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_EqualityExpression);
            list.push_back(lpg_NOT_EQUAL);
            list.push_back(lpg_RelationalExpression);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_EqualityExpression->accept(v);
                lpg_NOT_EQUAL->accept(v);
                lpg_RelationalExpression->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 434:  AssignmentOperator ::= =
     *</b>
     */
    struct AssignmentOperator0 :public AstToken
    {
        IToken* getEQUAL() { return leftIToken; }

        AssignmentOperator0(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 435:  AssignmentOperator ::= *=
     *</b>
     */
    struct AssignmentOperator1 :public AstToken
    {
        IToken* getMULTIPLY_EQUAL() { return leftIToken; }

        AssignmentOperator1(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 436:  AssignmentOperator ::= /=
     *</b>
     */
    struct AssignmentOperator2 :public AstToken
    {
        IToken* getDIVIDE_EQUAL() { return leftIToken; }

        AssignmentOperator2(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 437:  AssignmentOperator ::= %=
     *</b>
     */
    struct AssignmentOperator3 :public AstToken
    {
        IToken* getREMAINDER_EQUAL() { return leftIToken; }

        AssignmentOperator3(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 438:  AssignmentOperator ::= +=
     *</b>
     */
    struct AssignmentOperator4 :public AstToken
    {
        IToken* getPLUS_EQUAL() { return leftIToken; }

        AssignmentOperator4(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 439:  AssignmentOperator ::= -=
     *</b>
     */
    struct AssignmentOperator5 :public AstToken
    {
        IToken* getMINUS_EQUAL() { return leftIToken; }

        AssignmentOperator5(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 440:  AssignmentOperator ::= <<=
     *</b>
     */
    struct AssignmentOperator6 :public AstToken
    {
        IToken* getLEFT_SHIFT_EQUAL() { return leftIToken; }

        AssignmentOperator6(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 441:  AssignmentOperator ::= > > =
     *</b>
     */
    struct AssignmentOperator7 :public Ast
    {
        IAst* lpg_GREATER;
        IAst* lpg_GREATER2;
        IAst* lpg_EQUAL;

        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }
        IAst* getGREATER2() { return lpg_GREATER2; };
        void setGREATER2(IAst* lpg_GREATER2) { this->lpg_GREATER2 = lpg_GREATER2; }
        IAst* getEQUAL() { return lpg_EQUAL; };
        void setEQUAL(IAst* lpg_EQUAL) { this->lpg_EQUAL = lpg_EQUAL; }

        AssignmentOperator7(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_GREATER,
            IAst* lpg_GREATER2,
            IAst* lpg_EQUAL) :Ast(leftIToken, rightIToken) {
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            this->lpg_GREATER2 = lpg_GREATER2;
            ((Ast*)lpg_GREATER2)->setParent(this);
            this->lpg_EQUAL = lpg_EQUAL;
            ((Ast*)lpg_EQUAL)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_GREATER);
            list.push_back(lpg_GREATER2);
            list.push_back(lpg_EQUAL);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_GREATER->accept(v);
                lpg_GREATER2->accept(v);
                lpg_EQUAL->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 442:  AssignmentOperator ::= > > > =
     *</b>
     */
    struct AssignmentOperator8 :public Ast
    {
        IAst* lpg_GREATER;
        IAst* lpg_GREATER2;
        IAst* lpg_GREATER3;
        IAst* lpg_EQUAL;

        IAst* getGREATER() { return lpg_GREATER; };
        void setGREATER(IAst* lpg_GREATER) { this->lpg_GREATER = lpg_GREATER; }
        IAst* getGREATER2() { return lpg_GREATER2; };
        void setGREATER2(IAst* lpg_GREATER2) { this->lpg_GREATER2 = lpg_GREATER2; }
        IAst* getGREATER3() { return lpg_GREATER3; };
        void setGREATER3(IAst* lpg_GREATER3) { this->lpg_GREATER3 = lpg_GREATER3; }
        IAst* getEQUAL() { return lpg_EQUAL; };
        void setEQUAL(IAst* lpg_EQUAL) { this->lpg_EQUAL = lpg_EQUAL; }

        AssignmentOperator8(IToken* leftIToken, IToken* rightIToken,
            IAst* lpg_GREATER,
            IAst* lpg_GREATER2,
            IAst* lpg_GREATER3,
            IAst* lpg_EQUAL) :Ast(leftIToken, rightIToken) {
            this->lpg_GREATER = lpg_GREATER;
            ((Ast*)lpg_GREATER)->setParent(this);
            this->lpg_GREATER2 = lpg_GREATER2;
            ((Ast*)lpg_GREATER2)->setParent(this);
            this->lpg_GREATER3 = lpg_GREATER3;
            ((Ast*)lpg_GREATER3)->setParent(this);
            this->lpg_EQUAL = lpg_EQUAL;
            ((Ast*)lpg_EQUAL)->setParent(this);
            initialize();
        }

        /**
         * A list of all children of this node, including the nullptr ones.
         */
        std::vector<IAst*> getAllChildren()
        {
            std::vector<IAst*> list;
            list.push_back(lpg_GREATER);
            list.push_back(lpg_GREATER2);
            list.push_back(lpg_GREATER3);
            list.push_back(lpg_EQUAL);
            return list;
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            bool checkChildren = v->visit(this);
            if (checkChildren)
            {
                lpg_GREATER->accept(v);
                lpg_GREATER2->accept(v);
                lpg_GREATER3->accept(v);
                lpg_EQUAL->accept(v);
            }
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 443:  AssignmentOperator ::= &=
     *</b>
     */
    struct AssignmentOperator9 :public AstToken
    {
        IToken* getAND_EQUAL() { return leftIToken; }

        AssignmentOperator9(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 444:  AssignmentOperator ::= ^=
     *</b>
     */
    struct AssignmentOperator10 :public AstToken
    {
        IToken* getXOR_EQUAL() { return leftIToken; }

        AssignmentOperator10(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    /**
     *<b>
     *<li>Rule 445:  AssignmentOperator ::= |=
     *</b>
     */
    struct AssignmentOperator11 :public AstToken
    {
        IToken* getOR_EQUAL() { return leftIToken; }

        AssignmentOperator11(IToken* token) :AstToken(token)
        {
            initialize();
        }

        void accept(IAstVisitor* v)
        {
            if (!v->preVisit(this)) return;
            enter((Visitor*)v);
            v->postVisit(this);
        }

        void enter(Visitor* v)
        {
            v->visit(this);
            v->endVisit(this);
        }
    };

    struct Visitor :public IAstVisitor
    {
        virtual  bool visit(Ast* n) = 0;
        virtual   void endVisit(Ast* n) = 0;

        virtual   bool visit(AstToken* n) = 0;
        virtual  void endVisit(AstToken* n) = 0;

        virtual   bool visit(identifier* n) = 0;
        virtual  void endVisit(identifier* n) = 0;

        virtual   bool visit(PrimitiveType* n) = 0;
        virtual  void endVisit(PrimitiveType* n) = 0;

        virtual   bool visit(ClassType* n) = 0;
        virtual  void endVisit(ClassType* n) = 0;

        virtual   bool visit(InterfaceType* n) = 0;
        virtual  void endVisit(InterfaceType* n) = 0;

        virtual   bool visit(TypeName* n) = 0;
        virtual  void endVisit(TypeName* n) = 0;

        virtual   bool visit(ArrayType* n) = 0;
        virtual  void endVisit(ArrayType* n) = 0;

        virtual   bool visit(TypeParameter* n) = 0;
        virtual  void endVisit(TypeParameter* n) = 0;

        virtual   bool visit(TypeBound* n) = 0;
        virtual  void endVisit(TypeBound* n) = 0;

        virtual   bool visit(AdditionalBoundList* n) = 0;
        virtual  void endVisit(AdditionalBoundList* n) = 0;

        virtual   bool visit(AdditionalBound* n) = 0;
        virtual  void endVisit(AdditionalBound* n) = 0;

        virtual   bool visit(TypeArguments* n) = 0;
        virtual  void endVisit(TypeArguments* n) = 0;

        virtual   bool visit(ActualTypeArgumentList* n) = 0;
        virtual  void endVisit(ActualTypeArgumentList* n) = 0;

        virtual   bool visit(Wildcard* n) = 0;
        virtual  void endVisit(Wildcard* n) = 0;

        virtual   bool visit(PackageName* n) = 0;
        virtual  void endVisit(PackageName* n) = 0;

        virtual   bool visit(ExpressionName* n) = 0;
        virtual  void endVisit(ExpressionName* n) = 0;

        virtual   bool visit(MethodName* n) = 0;
        virtual  void endVisit(MethodName* n) = 0;

        virtual   bool visit(PackageOrTypeName* n) = 0;
        virtual  void endVisit(PackageOrTypeName* n) = 0;

        virtual   bool visit(AmbiguousName* n) = 0;
        virtual  void endVisit(AmbiguousName* n) = 0;

        virtual   bool visit(CompilationUnit* n) = 0;
        virtual  void endVisit(CompilationUnit* n) = 0;

        virtual   bool visit(ImportDeclarations* n) = 0;
        virtual  void endVisit(ImportDeclarations* n) = 0;

        virtual   bool visit(TypeDeclarations* n) = 0;
        virtual  void endVisit(TypeDeclarations* n) = 0;

        virtual   bool visit(PackageDeclaration* n) = 0;
        virtual  void endVisit(PackageDeclaration* n) = 0;

        virtual   bool visit(SingleTypeImportDeclaration* n) = 0;
        virtual  void endVisit(SingleTypeImportDeclaration* n) = 0;

        virtual   bool visit(TypeImportOnDemandDeclaration* n) = 0;
        virtual  void endVisit(TypeImportOnDemandDeclaration* n) = 0;

        virtual   bool visit(SingleStaticImportDeclaration* n) = 0;
        virtual  void endVisit(SingleStaticImportDeclaration* n) = 0;

        virtual   bool visit(StaticImportOnDemandDeclaration* n) = 0;
        virtual  void endVisit(StaticImportOnDemandDeclaration* n) = 0;

        virtual   bool visit(TypeDeclaration* n) = 0;
        virtual  void endVisit(TypeDeclaration* n) = 0;

        virtual   bool visit(NormalClassDeclaration* n) = 0;
        virtual  void endVisit(NormalClassDeclaration* n) = 0;

        virtual   bool visit(ClassModifiers* n) = 0;
        virtual  void endVisit(ClassModifiers* n) = 0;

        virtual   bool visit(TypeParameters* n) = 0;
        virtual  void endVisit(TypeParameters* n) = 0;

        virtual   bool visit(TypeParameterList* n) = 0;
        virtual  void endVisit(TypeParameterList* n) = 0;

        virtual   bool visit(Super* n) = 0;
        virtual  void endVisit(Super* n) = 0;

        virtual   bool visit(Interfaces* n) = 0;
        virtual  void endVisit(Interfaces* n) = 0;

        virtual   bool visit(InterfaceTypeList* n) = 0;
        virtual  void endVisit(InterfaceTypeList* n) = 0;

        virtual   bool visit(ClassBody* n) = 0;
        virtual  void endVisit(ClassBody* n) = 0;

        virtual   bool visit(ClassBodyDeclarations* n) = 0;
        virtual  void endVisit(ClassBodyDeclarations* n) = 0;

        virtual   bool visit(ClassMemberDeclaration* n) = 0;
        virtual  void endVisit(ClassMemberDeclaration* n) = 0;

        virtual   bool visit(FieldDeclaration* n) = 0;
        virtual  void endVisit(FieldDeclaration* n) = 0;

        virtual   bool visit(VariableDeclarators* n) = 0;
        virtual  void endVisit(VariableDeclarators* n) = 0;

        virtual   bool visit(VariableDeclarator* n) = 0;
        virtual  void endVisit(VariableDeclarator* n) = 0;

        virtual   bool visit(VariableDeclaratorId* n) = 0;
        virtual  void endVisit(VariableDeclaratorId* n) = 0;

        virtual   bool visit(FieldModifiers* n) = 0;
        virtual  void endVisit(FieldModifiers* n) = 0;

        virtual   bool visit(MethodDeclaration* n) = 0;
        virtual  void endVisit(MethodDeclaration* n) = 0;

        virtual   bool visit(MethodHeader* n) = 0;
        virtual  void endVisit(MethodHeader* n) = 0;

        virtual   bool visit(ResultType* n) = 0;
        virtual  void endVisit(ResultType* n) = 0;

        virtual   bool visit(FormalParameterList* n) = 0;
        virtual  void endVisit(FormalParameterList* n) = 0;

        virtual   bool visit(FormalParameters* n) = 0;
        virtual  void endVisit(FormalParameters* n) = 0;

        virtual   bool visit(FormalParameter* n) = 0;
        virtual  void endVisit(FormalParameter* n) = 0;

        virtual   bool visit(VariableModifiers* n) = 0;
        virtual  void endVisit(VariableModifiers* n) = 0;

        virtual   bool visit(VariableModifier* n) = 0;
        virtual  void endVisit(VariableModifier* n) = 0;

        virtual   bool visit(LastFormalParameter* n) = 0;
        virtual  void endVisit(LastFormalParameter* n) = 0;

        virtual   bool visit(MethodModifiers* n) = 0;
        virtual  void endVisit(MethodModifiers* n) = 0;

        virtual   bool visit(Throws* n) = 0;
        virtual  void endVisit(Throws* n) = 0;

        virtual   bool visit(ExceptionTypeList* n) = 0;
        virtual  void endVisit(ExceptionTypeList* n) = 0;

        virtual   bool visit(MethodBody* n) = 0;
        virtual  void endVisit(MethodBody* n) = 0;

        virtual   bool visit(StaticInitializer* n) = 0;
        virtual  void endVisit(StaticInitializer* n) = 0;

        virtual   bool visit(ConstructorDeclaration* n) = 0;
        virtual  void endVisit(ConstructorDeclaration* n) = 0;

        virtual   bool visit(ConstructorDeclarator* n) = 0;
        virtual  void endVisit(ConstructorDeclarator* n) = 0;

        virtual   bool visit(ConstructorModifiers* n) = 0;
        virtual  void endVisit(ConstructorModifiers* n) = 0;

        virtual   bool visit(ConstructorBody* n) = 0;
        virtual  void endVisit(ConstructorBody* n) = 0;

        virtual   bool visit(EnumDeclaration* n) = 0;
        virtual  void endVisit(EnumDeclaration* n) = 0;

        virtual   bool visit(EnumBody* n) = 0;
        virtual  void endVisit(EnumBody* n) = 0;

        virtual   bool visit(EnumConstants* n) = 0;
        virtual  void endVisit(EnumConstants* n) = 0;

        virtual   bool visit(EnumConstant* n) = 0;
        virtual  void endVisit(EnumConstant* n) = 0;

        virtual   bool visit(Arguments* n) = 0;
        virtual  void endVisit(Arguments* n) = 0;

        virtual   bool visit(EnumBodyDeclarations* n) = 0;
        virtual  void endVisit(EnumBodyDeclarations* n) = 0;

        virtual   bool visit(NormalInterfaceDeclaration* n) = 0;
        virtual  void endVisit(NormalInterfaceDeclaration* n) = 0;

        virtual   bool visit(InterfaceModifiers* n) = 0;
        virtual  void endVisit(InterfaceModifiers* n) = 0;

        virtual   bool visit(InterfaceBody* n) = 0;
        virtual  void endVisit(InterfaceBody* n) = 0;

        virtual   bool visit(InterfaceMemberDeclarations* n) = 0;
        virtual  void endVisit(InterfaceMemberDeclarations* n) = 0;

        virtual   bool visit(InterfaceMemberDeclaration* n) = 0;
        virtual  void endVisit(InterfaceMemberDeclaration* n) = 0;

        virtual   bool visit(ConstantDeclaration* n) = 0;
        virtual  void endVisit(ConstantDeclaration* n) = 0;

        virtual   bool visit(ConstantModifiers* n) = 0;
        virtual  void endVisit(ConstantModifiers* n) = 0;

        virtual   bool visit(AbstractMethodDeclaration* n) = 0;
        virtual  void endVisit(AbstractMethodDeclaration* n) = 0;

        virtual   bool visit(AbstractMethodModifiers* n) = 0;
        virtual  void endVisit(AbstractMethodModifiers* n) = 0;

        virtual   bool visit(AnnotationTypeDeclaration* n) = 0;
        virtual  void endVisit(AnnotationTypeDeclaration* n) = 0;

        virtual   bool visit(AnnotationTypeBody* n) = 0;
        virtual  void endVisit(AnnotationTypeBody* n) = 0;

        virtual   bool visit(AnnotationTypeElementDeclarations* n) = 0;
        virtual  void endVisit(AnnotationTypeElementDeclarations* n) = 0;

        virtual   bool visit(DefaultValue* n) = 0;
        virtual  void endVisit(DefaultValue* n) = 0;

        virtual   bool visit(Annotations* n) = 0;
        virtual  void endVisit(Annotations* n) = 0;

        virtual   bool visit(NormalAnnotation* n) = 0;
        virtual  void endVisit(NormalAnnotation* n) = 0;

        virtual   bool visit(ElementValuePairs* n) = 0;
        virtual  void endVisit(ElementValuePairs* n) = 0;

        virtual   bool visit(ElementValuePair* n) = 0;
        virtual  void endVisit(ElementValuePair* n) = 0;

        virtual   bool visit(ElementValueArrayInitializer* n) = 0;
        virtual  void endVisit(ElementValueArrayInitializer* n) = 0;

        virtual   bool visit(ElementValues* n) = 0;
        virtual  void endVisit(ElementValues* n) = 0;

        virtual   bool visit(MarkerAnnotation* n) = 0;
        virtual  void endVisit(MarkerAnnotation* n) = 0;

        virtual   bool visit(SingleElementAnnotation* n) = 0;
        virtual  void endVisit(SingleElementAnnotation* n) = 0;

        virtual   bool visit(ArrayInitializer* n) = 0;
        virtual  void endVisit(ArrayInitializer* n) = 0;

        virtual   bool visit(VariableInitializers* n) = 0;
        virtual  void endVisit(VariableInitializers* n) = 0;

        virtual   bool visit(Block* n) = 0;
        virtual  void endVisit(Block* n) = 0;

        virtual   bool visit(BlockStatements* n) = 0;
        virtual  void endVisit(BlockStatements* n) = 0;

        virtual   bool visit(LocalVariableDeclarationStatement* n) = 0;
        virtual  void endVisit(LocalVariableDeclarationStatement* n) = 0;

        virtual   bool visit(LocalVariableDeclaration* n) = 0;
        virtual  void endVisit(LocalVariableDeclaration* n) = 0;

        virtual   bool visit(IfThenStatement* n) = 0;
        virtual  void endVisit(IfThenStatement* n) = 0;

        virtual   bool visit(IfThenElseStatement* n) = 0;
        virtual  void endVisit(IfThenElseStatement* n) = 0;

        virtual   bool visit(IfThenElseStatementNoShortIf* n) = 0;
        virtual  void endVisit(IfThenElseStatementNoShortIf* n) = 0;

        virtual   bool visit(EmptyStatement* n) = 0;
        virtual  void endVisit(EmptyStatement* n) = 0;

        virtual   bool visit(LabeledStatement* n) = 0;
        virtual  void endVisit(LabeledStatement* n) = 0;

        virtual   bool visit(LabeledStatementNoShortIf* n) = 0;
        virtual  void endVisit(LabeledStatementNoShortIf* n) = 0;

        virtual   bool visit(ExpressionStatement* n) = 0;
        virtual  void endVisit(ExpressionStatement* n) = 0;

        virtual   bool visit(SwitchStatement* n) = 0;
        virtual  void endVisit(SwitchStatement* n) = 0;

        virtual   bool visit(SwitchBlock* n) = 0;
        virtual  void endVisit(SwitchBlock* n) = 0;

        virtual   bool visit(SwitchBlockStatementGroups* n) = 0;
        virtual  void endVisit(SwitchBlockStatementGroups* n) = 0;

        virtual   bool visit(SwitchBlockStatementGroup* n) = 0;
        virtual  void endVisit(SwitchBlockStatementGroup* n) = 0;

        virtual   bool visit(SwitchLabels* n) = 0;
        virtual  void endVisit(SwitchLabels* n) = 0;

        virtual   bool visit(WhileStatement* n) = 0;
        virtual  void endVisit(WhileStatement* n) = 0;

        virtual   bool visit(WhileStatementNoShortIf* n) = 0;
        virtual  void endVisit(WhileStatementNoShortIf* n) = 0;

        virtual   bool visit(DoStatement* n) = 0;
        virtual  void endVisit(DoStatement* n) = 0;

        virtual   bool visit(BasicForStatement* n) = 0;
        virtual  void endVisit(BasicForStatement* n) = 0;

        virtual   bool visit(ForStatementNoShortIf* n) = 0;
        virtual  void endVisit(ForStatementNoShortIf* n) = 0;

        virtual   bool visit(StatementExpressionList* n) = 0;
        virtual  void endVisit(StatementExpressionList* n) = 0;

        virtual   bool visit(EnhancedForStatement* n) = 0;
        virtual  void endVisit(EnhancedForStatement* n) = 0;

        virtual   bool visit(BreakStatement* n) = 0;
        virtual  void endVisit(BreakStatement* n) = 0;

        virtual   bool visit(ContinueStatement* n) = 0;
        virtual  void endVisit(ContinueStatement* n) = 0;

        virtual   bool visit(ReturnStatement* n) = 0;
        virtual  void endVisit(ReturnStatement* n) = 0;

        virtual   bool visit(ThrowStatement* n) = 0;
        virtual  void endVisit(ThrowStatement* n) = 0;

        virtual   bool visit(SynchronizedStatement* n) = 0;
        virtual  void endVisit(SynchronizedStatement* n) = 0;

        virtual   bool visit(Catches* n) = 0;
        virtual  void endVisit(Catches* n) = 0;

        virtual   bool visit(CatchClause* n) = 0;
        virtual  void endVisit(CatchClause* n) = 0;

        virtual   bool visit(Finally* n) = 0;
        virtual  void endVisit(Finally* n) = 0;

        virtual   bool visit(ArgumentList* n) = 0;
        virtual  void endVisit(ArgumentList* n) = 0;

        virtual   bool visit(DimExprs* n) = 0;
        virtual  void endVisit(DimExprs* n) = 0;

        virtual   bool visit(DimExpr* n) = 0;
        virtual  void endVisit(DimExpr* n) = 0;

        virtual   bool visit(PostIncrementExpression* n) = 0;
        virtual  void endVisit(PostIncrementExpression* n) = 0;

        virtual   bool visit(PostDecrementExpression* n) = 0;
        virtual  void endVisit(PostDecrementExpression* n) = 0;

        virtual   bool visit(PreIncrementExpression* n) = 0;
        virtual  void endVisit(PreIncrementExpression* n) = 0;

        virtual   bool visit(PreDecrementExpression* n) = 0;
        virtual  void endVisit(PreDecrementExpression* n) = 0;

        virtual   bool visit(AndExpression* n) = 0;
        virtual  void endVisit(AndExpression* n) = 0;

        virtual   bool visit(ExclusiveOrExpression* n) = 0;
        virtual  void endVisit(ExclusiveOrExpression* n) = 0;

        virtual   bool visit(InclusiveOrExpression* n) = 0;
        virtual  void endVisit(InclusiveOrExpression* n) = 0;

        virtual   bool visit(ConditionalAndExpression* n) = 0;
        virtual  void endVisit(ConditionalAndExpression* n) = 0;

        virtual   bool visit(ConditionalOrExpression* n) = 0;
        virtual  void endVisit(ConditionalOrExpression* n) = 0;

        virtual   bool visit(ConditionalExpression* n) = 0;
        virtual  void endVisit(ConditionalExpression* n) = 0;

        virtual   bool visit(Assignment* n) = 0;
        virtual  void endVisit(Assignment* n) = 0;

        virtual   bool visit(Commaopt* n) = 0;
        virtual  void endVisit(Commaopt* n) = 0;

        virtual   bool visit(Ellipsisopt* n) = 0;
        virtual  void endVisit(Ellipsisopt* n) = 0;

        virtual   bool visit(LPGUserAction0* n) = 0;
        virtual  void endVisit(LPGUserAction0* n) = 0;

        virtual   bool visit(LPGUserAction1* n) = 0;
        virtual  void endVisit(LPGUserAction1* n) = 0;

        virtual   bool visit(LPGUserAction2* n) = 0;
        virtual  void endVisit(LPGUserAction2* n) = 0;

        virtual   bool visit(LPGUserAction3* n) = 0;
        virtual  void endVisit(LPGUserAction3* n) = 0;

        virtual   bool visit(LPGUserAction4* n) = 0;
        virtual  void endVisit(LPGUserAction4* n) = 0;

        virtual   bool visit(IntegralType0* n) = 0;
        virtual  void endVisit(IntegralType0* n) = 0;

        virtual   bool visit(IntegralType1* n) = 0;
        virtual  void endVisit(IntegralType1* n) = 0;

        virtual   bool visit(IntegralType2* n) = 0;
        virtual  void endVisit(IntegralType2* n) = 0;

        virtual   bool visit(IntegralType3* n) = 0;
        virtual  void endVisit(IntegralType3* n) = 0;

        virtual   bool visit(IntegralType4* n) = 0;
        virtual  void endVisit(IntegralType4* n) = 0;

        virtual   bool visit(FloatingPointType0* n) = 0;
        virtual  void endVisit(FloatingPointType0* n) = 0;

        virtual   bool visit(FloatingPointType1* n) = 0;
        virtual  void endVisit(FloatingPointType1* n) = 0;

        virtual   bool visit(WildcardBounds0* n) = 0;
        virtual  void endVisit(WildcardBounds0* n) = 0;

        virtual   bool visit(WildcardBounds1* n) = 0;
        virtual  void endVisit(WildcardBounds1* n) = 0;

        virtual   bool visit(ClassModifier0* n) = 0;
        virtual  void endVisit(ClassModifier0* n) = 0;

        virtual   bool visit(ClassModifier1* n) = 0;
        virtual  void endVisit(ClassModifier1* n) = 0;

        virtual   bool visit(ClassModifier2* n) = 0;
        virtual  void endVisit(ClassModifier2* n) = 0;

        virtual   bool visit(ClassModifier3* n) = 0;
        virtual  void endVisit(ClassModifier3* n) = 0;

        virtual   bool visit(ClassModifier4* n) = 0;
        virtual  void endVisit(ClassModifier4* n) = 0;

        virtual   bool visit(ClassModifier5* n) = 0;
        virtual  void endVisit(ClassModifier5* n) = 0;

        virtual   bool visit(ClassModifier6* n) = 0;
        virtual  void endVisit(ClassModifier6* n) = 0;

        virtual   bool visit(FieldModifier0* n) = 0;
        virtual  void endVisit(FieldModifier0* n) = 0;

        virtual   bool visit(FieldModifier1* n) = 0;
        virtual  void endVisit(FieldModifier1* n) = 0;

        virtual   bool visit(FieldModifier2* n) = 0;
        virtual  void endVisit(FieldModifier2* n) = 0;

        virtual   bool visit(FieldModifier3* n) = 0;
        virtual  void endVisit(FieldModifier3* n) = 0;

        virtual   bool visit(FieldModifier4* n) = 0;
        virtual  void endVisit(FieldModifier4* n) = 0;

        virtual   bool visit(FieldModifier5* n) = 0;
        virtual  void endVisit(FieldModifier5* n) = 0;

        virtual   bool visit(FieldModifier6* n) = 0;
        virtual  void endVisit(FieldModifier6* n) = 0;

        virtual   bool visit(MethodDeclarator0* n) = 0;
        virtual  void endVisit(MethodDeclarator0* n) = 0;

        virtual   bool visit(MethodDeclarator1* n) = 0;
        virtual  void endVisit(MethodDeclarator1* n) = 0;

        virtual   bool visit(MethodModifier0* n) = 0;
        virtual  void endVisit(MethodModifier0* n) = 0;

        virtual   bool visit(MethodModifier1* n) = 0;
        virtual  void endVisit(MethodModifier1* n) = 0;

        virtual   bool visit(MethodModifier2* n) = 0;
        virtual  void endVisit(MethodModifier2* n) = 0;

        virtual   bool visit(MethodModifier3* n) = 0;
        virtual  void endVisit(MethodModifier3* n) = 0;

        virtual   bool visit(MethodModifier4* n) = 0;
        virtual  void endVisit(MethodModifier4* n) = 0;

        virtual   bool visit(MethodModifier5* n) = 0;
        virtual  void endVisit(MethodModifier5* n) = 0;

        virtual   bool visit(MethodModifier6* n) = 0;
        virtual  void endVisit(MethodModifier6* n) = 0;

        virtual   bool visit(MethodModifier7* n) = 0;
        virtual  void endVisit(MethodModifier7* n) = 0;

        virtual   bool visit(MethodModifier8* n) = 0;
        virtual  void endVisit(MethodModifier8* n) = 0;

        virtual   bool visit(ConstructorModifier0* n) = 0;
        virtual  void endVisit(ConstructorModifier0* n) = 0;

        virtual   bool visit(ConstructorModifier1* n) = 0;
        virtual  void endVisit(ConstructorModifier1* n) = 0;

        virtual   bool visit(ConstructorModifier2* n) = 0;
        virtual  void endVisit(ConstructorModifier2* n) = 0;

        virtual   bool visit(ExplicitConstructorInvocation0* n) = 0;
        virtual  void endVisit(ExplicitConstructorInvocation0* n) = 0;

        virtual   bool visit(ExplicitConstructorInvocation1* n) = 0;
        virtual  void endVisit(ExplicitConstructorInvocation1* n) = 0;

        virtual   bool visit(ExplicitConstructorInvocation2* n) = 0;
        virtual  void endVisit(ExplicitConstructorInvocation2* n) = 0;

        virtual   bool visit(InterfaceModifier0* n) = 0;
        virtual  void endVisit(InterfaceModifier0* n) = 0;

        virtual   bool visit(InterfaceModifier1* n) = 0;
        virtual  void endVisit(InterfaceModifier1* n) = 0;

        virtual   bool visit(InterfaceModifier2* n) = 0;
        virtual  void endVisit(InterfaceModifier2* n) = 0;

        virtual   bool visit(InterfaceModifier3* n) = 0;
        virtual  void endVisit(InterfaceModifier3* n) = 0;

        virtual   bool visit(InterfaceModifier4* n) = 0;
        virtual  void endVisit(InterfaceModifier4* n) = 0;

        virtual   bool visit(InterfaceModifier5* n) = 0;
        virtual  void endVisit(InterfaceModifier5* n) = 0;

        virtual   bool visit(ExtendsInterfaces0* n) = 0;
        virtual  void endVisit(ExtendsInterfaces0* n) = 0;

        virtual   bool visit(ExtendsInterfaces1* n) = 0;
        virtual  void endVisit(ExtendsInterfaces1* n) = 0;

        virtual   bool visit(ConstantModifier0* n) = 0;
        virtual  void endVisit(ConstantModifier0* n) = 0;

        virtual   bool visit(ConstantModifier1* n) = 0;
        virtual  void endVisit(ConstantModifier1* n) = 0;

        virtual   bool visit(ConstantModifier2* n) = 0;
        virtual  void endVisit(ConstantModifier2* n) = 0;

        virtual   bool visit(AbstractMethodModifier0* n) = 0;
        virtual  void endVisit(AbstractMethodModifier0* n) = 0;

        virtual   bool visit(AbstractMethodModifier1* n) = 0;
        virtual  void endVisit(AbstractMethodModifier1* n) = 0;

        virtual   bool visit(AnnotationTypeElementDeclaration0* n) = 0;
        virtual  void endVisit(AnnotationTypeElementDeclaration0* n) = 0;

        virtual   bool visit(AnnotationTypeElementDeclaration1* n) = 0;
        virtual  void endVisit(AnnotationTypeElementDeclaration1* n) = 0;

        virtual   bool visit(AssertStatement0* n) = 0;
        virtual  void endVisit(AssertStatement0* n) = 0;

        virtual   bool visit(AssertStatement1* n) = 0;
        virtual  void endVisit(AssertStatement1* n) = 0;

        virtual   bool visit(SwitchLabel0* n) = 0;
        virtual  void endVisit(SwitchLabel0* n) = 0;

        virtual   bool visit(SwitchLabel1* n) = 0;
        virtual  void endVisit(SwitchLabel1* n) = 0;

        virtual   bool visit(SwitchLabel2* n) = 0;
        virtual  void endVisit(SwitchLabel2* n) = 0;

        virtual   bool visit(TryStatement0* n) = 0;
        virtual  void endVisit(TryStatement0* n) = 0;

        virtual   bool visit(TryStatement1* n) = 0;
        virtual  void endVisit(TryStatement1* n) = 0;

        virtual   bool visit(PrimaryNoNewArray0* n) = 0;
        virtual  void endVisit(PrimaryNoNewArray0* n) = 0;

        virtual   bool visit(PrimaryNoNewArray1* n) = 0;
        virtual  void endVisit(PrimaryNoNewArray1* n) = 0;

        virtual   bool visit(PrimaryNoNewArray2* n) = 0;
        virtual  void endVisit(PrimaryNoNewArray2* n) = 0;

        virtual   bool visit(PrimaryNoNewArray3* n) = 0;
        virtual  void endVisit(PrimaryNoNewArray3* n) = 0;

        virtual   bool visit(PrimaryNoNewArray4* n) = 0;
        virtual  void endVisit(PrimaryNoNewArray4* n) = 0;

        virtual   bool visit(Literal0* n) = 0;
        virtual  void endVisit(Literal0* n) = 0;

        virtual   bool visit(Literal1* n) = 0;
        virtual  void endVisit(Literal1* n) = 0;

        virtual   bool visit(Literal2* n) = 0;
        virtual  void endVisit(Literal2* n) = 0;

        virtual   bool visit(Literal3* n) = 0;
        virtual  void endVisit(Literal3* n) = 0;

        virtual   bool visit(Literal4* n) = 0;
        virtual  void endVisit(Literal4* n) = 0;

        virtual   bool visit(Literal5* n) = 0;
        virtual  void endVisit(Literal5* n) = 0;

        virtual   bool visit(Literal6* n) = 0;
        virtual  void endVisit(Literal6* n) = 0;

        virtual   bool visit(BooleanLiteral0* n) = 0;
        virtual  void endVisit(BooleanLiteral0* n) = 0;

        virtual   bool visit(BooleanLiteral1* n) = 0;
        virtual  void endVisit(BooleanLiteral1* n) = 0;

        virtual   bool visit(ClassInstanceCreationExpression0* n) = 0;
        virtual  void endVisit(ClassInstanceCreationExpression0* n) = 0;

        virtual   bool visit(ClassInstanceCreationExpression1* n) = 0;
        virtual  void endVisit(ClassInstanceCreationExpression1* n) = 0;

        virtual   bool visit(ArrayCreationExpression0* n) = 0;
        virtual  void endVisit(ArrayCreationExpression0* n) = 0;

        virtual   bool visit(ArrayCreationExpression1* n) = 0;
        virtual  void endVisit(ArrayCreationExpression1* n) = 0;

        virtual   bool visit(ArrayCreationExpression2* n) = 0;
        virtual  void endVisit(ArrayCreationExpression2* n) = 0;

        virtual   bool visit(ArrayCreationExpression3* n) = 0;
        virtual  void endVisit(ArrayCreationExpression3* n) = 0;

        virtual   bool visit(Dims0* n) = 0;
        virtual  void endVisit(Dims0* n) = 0;

        virtual   bool visit(Dims1* n) = 0;
        virtual  void endVisit(Dims1* n) = 0;

        virtual   bool visit(FieldAccess0* n) = 0;
        virtual  void endVisit(FieldAccess0* n) = 0;

        virtual   bool visit(FieldAccess1* n) = 0;
        virtual  void endVisit(FieldAccess1* n) = 0;

        virtual   bool visit(FieldAccess2* n) = 0;
        virtual  void endVisit(FieldAccess2* n) = 0;

        virtual   bool visit(MethodInvocation0* n) = 0;
        virtual  void endVisit(MethodInvocation0* n) = 0;

        virtual   bool visit(MethodInvocation1* n) = 0;
        virtual  void endVisit(MethodInvocation1* n) = 0;

        virtual   bool visit(MethodInvocation2* n) = 0;
        virtual  void endVisit(MethodInvocation2* n) = 0;

        virtual   bool visit(MethodInvocation3* n) = 0;
        virtual  void endVisit(MethodInvocation3* n) = 0;

        virtual   bool visit(MethodInvocation4* n) = 0;
        virtual  void endVisit(MethodInvocation4* n) = 0;

        virtual   bool visit(ArrayAccess0* n) = 0;
        virtual  void endVisit(ArrayAccess0* n) = 0;

        virtual   bool visit(ArrayAccess1* n) = 0;
        virtual  void endVisit(ArrayAccess1* n) = 0;

        virtual   bool visit(UnaryExpression0* n) = 0;
        virtual  void endVisit(UnaryExpression0* n) = 0;

        virtual   bool visit(UnaryExpression1* n) = 0;
        virtual  void endVisit(UnaryExpression1* n) = 0;

        virtual   bool visit(UnaryExpressionNotPlusMinus0* n) = 0;
        virtual  void endVisit(UnaryExpressionNotPlusMinus0* n) = 0;

        virtual   bool visit(UnaryExpressionNotPlusMinus1* n) = 0;
        virtual  void endVisit(UnaryExpressionNotPlusMinus1* n) = 0;

        virtual   bool visit(CastExpression0* n) = 0;
        virtual  void endVisit(CastExpression0* n) = 0;

        virtual   bool visit(CastExpression1* n) = 0;
        virtual  void endVisit(CastExpression1* n) = 0;

        virtual   bool visit(MultiplicativeExpression0* n) = 0;
        virtual  void endVisit(MultiplicativeExpression0* n) = 0;

        virtual   bool visit(MultiplicativeExpression1* n) = 0;
        virtual  void endVisit(MultiplicativeExpression1* n) = 0;

        virtual   bool visit(MultiplicativeExpression2* n) = 0;
        virtual  void endVisit(MultiplicativeExpression2* n) = 0;

        virtual   bool visit(AdditiveExpression0* n) = 0;
        virtual  void endVisit(AdditiveExpression0* n) = 0;

        virtual   bool visit(AdditiveExpression1* n) = 0;
        virtual  void endVisit(AdditiveExpression1* n) = 0;

        virtual   bool visit(ShiftExpression0* n) = 0;
        virtual  void endVisit(ShiftExpression0* n) = 0;

        virtual   bool visit(ShiftExpression1* n) = 0;
        virtual  void endVisit(ShiftExpression1* n) = 0;

        virtual   bool visit(ShiftExpression2* n) = 0;
        virtual  void endVisit(ShiftExpression2* n) = 0;

        virtual   bool visit(RelationalExpression0* n) = 0;
        virtual  void endVisit(RelationalExpression0* n) = 0;

        virtual   bool visit(RelationalExpression1* n) = 0;
        virtual  void endVisit(RelationalExpression1* n) = 0;

        virtual   bool visit(RelationalExpression2* n) = 0;
        virtual  void endVisit(RelationalExpression2* n) = 0;

        virtual   bool visit(RelationalExpression3* n) = 0;
        virtual  void endVisit(RelationalExpression3* n) = 0;

        virtual   bool visit(RelationalExpression4* n) = 0;
        virtual  void endVisit(RelationalExpression4* n) = 0;

        virtual   bool visit(EqualityExpression0* n) = 0;
        virtual  void endVisit(EqualityExpression0* n) = 0;

        virtual   bool visit(EqualityExpression1* n) = 0;
        virtual  void endVisit(EqualityExpression1* n) = 0;

        virtual   bool visit(AssignmentOperator0* n) = 0;
        virtual  void endVisit(AssignmentOperator0* n) = 0;

        virtual   bool visit(AssignmentOperator1* n) = 0;
        virtual  void endVisit(AssignmentOperator1* n) = 0;

        virtual   bool visit(AssignmentOperator2* n) = 0;
        virtual  void endVisit(AssignmentOperator2* n) = 0;

        virtual   bool visit(AssignmentOperator3* n) = 0;
        virtual  void endVisit(AssignmentOperator3* n) = 0;

        virtual   bool visit(AssignmentOperator4* n) = 0;
        virtual  void endVisit(AssignmentOperator4* n) = 0;

        virtual   bool visit(AssignmentOperator5* n) = 0;
        virtual  void endVisit(AssignmentOperator5* n) = 0;

        virtual   bool visit(AssignmentOperator6* n) = 0;
        virtual  void endVisit(AssignmentOperator6* n) = 0;

        virtual   bool visit(AssignmentOperator7* n) = 0;
        virtual  void endVisit(AssignmentOperator7* n) = 0;

        virtual   bool visit(AssignmentOperator8* n) = 0;
        virtual  void endVisit(AssignmentOperator8* n) = 0;

        virtual   bool visit(AssignmentOperator9* n) = 0;
        virtual  void endVisit(AssignmentOperator9* n) = 0;

        virtual   bool visit(AssignmentOperator10* n) = 0;
        virtual  void endVisit(AssignmentOperator10* n) = 0;

        virtual   bool visit(AssignmentOperator11* n) = 0;
        virtual  void endVisit(AssignmentOperator11* n) = 0;

    };

    struct AbstractVisitor :public Visitor
    {
        virtual void unimplementedVisitor(const std::string& s) = 0;

        virtual bool preVisit(IAst* element) { return true; }

        virtual void postVisit(IAst* element) {}

        virtual   bool visit(AstToken* n) { unimplementedVisitor("visit(AstToken)"); return true; }
        virtual  void endVisit(AstToken* n) { unimplementedVisitor("endVisit(AstToken)"); }

        virtual   bool visit(identifier* n) { unimplementedVisitor("visit(identifier)"); return true; }
        virtual  void endVisit(identifier* n) { unimplementedVisitor("endVisit(identifier)"); }

        virtual   bool visit(PrimitiveType* n) { unimplementedVisitor("visit(PrimitiveType)"); return true; }
        virtual  void endVisit(PrimitiveType* n) { unimplementedVisitor("endVisit(PrimitiveType)"); }

        virtual   bool visit(ClassType* n) { unimplementedVisitor("visit(ClassType)"); return true; }
        virtual  void endVisit(ClassType* n) { unimplementedVisitor("endVisit(ClassType)"); }

        virtual   bool visit(InterfaceType* n) { unimplementedVisitor("visit(InterfaceType)"); return true; }
        virtual  void endVisit(InterfaceType* n) { unimplementedVisitor("endVisit(InterfaceType)"); }

        virtual   bool visit(TypeName* n) { unimplementedVisitor("visit(TypeName)"); return true; }
        virtual  void endVisit(TypeName* n) { unimplementedVisitor("endVisit(TypeName)"); }

        virtual   bool visit(ArrayType* n) { unimplementedVisitor("visit(ArrayType)"); return true; }
        virtual  void endVisit(ArrayType* n) { unimplementedVisitor("endVisit(ArrayType)"); }

        virtual   bool visit(TypeParameter* n) { unimplementedVisitor("visit(TypeParameter)"); return true; }
        virtual  void endVisit(TypeParameter* n) { unimplementedVisitor("endVisit(TypeParameter)"); }

        virtual   bool visit(TypeBound* n) { unimplementedVisitor("visit(TypeBound)"); return true; }
        virtual  void endVisit(TypeBound* n) { unimplementedVisitor("endVisit(TypeBound)"); }

        virtual   bool visit(AdditionalBoundList* n) { unimplementedVisitor("visit(AdditionalBoundList)"); return true; }
        virtual  void endVisit(AdditionalBoundList* n) { unimplementedVisitor("endVisit(AdditionalBoundList)"); }

        virtual   bool visit(AdditionalBound* n) { unimplementedVisitor("visit(AdditionalBound)"); return true; }
        virtual  void endVisit(AdditionalBound* n) { unimplementedVisitor("endVisit(AdditionalBound)"); }

        virtual   bool visit(TypeArguments* n) { unimplementedVisitor("visit(TypeArguments)"); return true; }
        virtual  void endVisit(TypeArguments* n) { unimplementedVisitor("endVisit(TypeArguments)"); }

        virtual   bool visit(ActualTypeArgumentList* n) { unimplementedVisitor("visit(ActualTypeArgumentList)"); return true; }
        virtual  void endVisit(ActualTypeArgumentList* n) { unimplementedVisitor("endVisit(ActualTypeArgumentList)"); }

        virtual   bool visit(Wildcard* n) { unimplementedVisitor("visit(Wildcard)"); return true; }
        virtual  void endVisit(Wildcard* n) { unimplementedVisitor("endVisit(Wildcard)"); }

        virtual   bool visit(PackageName* n) { unimplementedVisitor("visit(PackageName)"); return true; }
        virtual  void endVisit(PackageName* n) { unimplementedVisitor("endVisit(PackageName)"); }

        virtual   bool visit(ExpressionName* n) { unimplementedVisitor("visit(ExpressionName)"); return true; }
        virtual  void endVisit(ExpressionName* n) { unimplementedVisitor("endVisit(ExpressionName)"); }

        virtual   bool visit(MethodName* n) { unimplementedVisitor("visit(MethodName)"); return true; }
        virtual  void endVisit(MethodName* n) { unimplementedVisitor("endVisit(MethodName)"); }

        virtual   bool visit(PackageOrTypeName* n) { unimplementedVisitor("visit(PackageOrTypeName)"); return true; }
        virtual  void endVisit(PackageOrTypeName* n) { unimplementedVisitor("endVisit(PackageOrTypeName)"); }

        virtual   bool visit(AmbiguousName* n) { unimplementedVisitor("visit(AmbiguousName)"); return true; }
        virtual  void endVisit(AmbiguousName* n) { unimplementedVisitor("endVisit(AmbiguousName)"); }

        virtual   bool visit(CompilationUnit* n) { unimplementedVisitor("visit(CompilationUnit)"); return true; }
        virtual  void endVisit(CompilationUnit* n) { unimplementedVisitor("endVisit(CompilationUnit)"); }

        virtual   bool visit(ImportDeclarations* n) { unimplementedVisitor("visit(ImportDeclarations)"); return true; }
        virtual  void endVisit(ImportDeclarations* n) { unimplementedVisitor("endVisit(ImportDeclarations)"); }

        virtual   bool visit(TypeDeclarations* n) { unimplementedVisitor("visit(TypeDeclarations)"); return true; }
        virtual  void endVisit(TypeDeclarations* n) { unimplementedVisitor("endVisit(TypeDeclarations)"); }

        virtual   bool visit(PackageDeclaration* n) { unimplementedVisitor("visit(PackageDeclaration)"); return true; }
        virtual  void endVisit(PackageDeclaration* n) { unimplementedVisitor("endVisit(PackageDeclaration)"); }

        virtual   bool visit(SingleTypeImportDeclaration* n) { unimplementedVisitor("visit(SingleTypeImportDeclaration)"); return true; }
        virtual  void endVisit(SingleTypeImportDeclaration* n) { unimplementedVisitor("endVisit(SingleTypeImportDeclaration)"); }

        virtual   bool visit(TypeImportOnDemandDeclaration* n) { unimplementedVisitor("visit(TypeImportOnDemandDeclaration)"); return true; }
        virtual  void endVisit(TypeImportOnDemandDeclaration* n) { unimplementedVisitor("endVisit(TypeImportOnDemandDeclaration)"); }

        virtual   bool visit(SingleStaticImportDeclaration* n) { unimplementedVisitor("visit(SingleStaticImportDeclaration)"); return true; }
        virtual  void endVisit(SingleStaticImportDeclaration* n) { unimplementedVisitor("endVisit(SingleStaticImportDeclaration)"); }

        virtual   bool visit(StaticImportOnDemandDeclaration* n) { unimplementedVisitor("visit(StaticImportOnDemandDeclaration)"); return true; }
        virtual  void endVisit(StaticImportOnDemandDeclaration* n) { unimplementedVisitor("endVisit(StaticImportOnDemandDeclaration)"); }

        virtual   bool visit(TypeDeclaration* n) { unimplementedVisitor("visit(TypeDeclaration)"); return true; }
        virtual  void endVisit(TypeDeclaration* n) { unimplementedVisitor("endVisit(TypeDeclaration)"); }

        virtual   bool visit(NormalClassDeclaration* n) { unimplementedVisitor("visit(NormalClassDeclaration)"); return true; }
        virtual  void endVisit(NormalClassDeclaration* n) { unimplementedVisitor("endVisit(NormalClassDeclaration)"); }

        virtual   bool visit(ClassModifiers* n) { unimplementedVisitor("visit(ClassModifiers)"); return true; }
        virtual  void endVisit(ClassModifiers* n) { unimplementedVisitor("endVisit(ClassModifiers)"); }

        virtual   bool visit(TypeParameters* n) { unimplementedVisitor("visit(TypeParameters)"); return true; }
        virtual  void endVisit(TypeParameters* n) { unimplementedVisitor("endVisit(TypeParameters)"); }

        virtual   bool visit(TypeParameterList* n) { unimplementedVisitor("visit(TypeParameterList)"); return true; }
        virtual  void endVisit(TypeParameterList* n) { unimplementedVisitor("endVisit(TypeParameterList)"); }

        virtual   bool visit(Super* n) { unimplementedVisitor("visit(Super)"); return true; }
        virtual  void endVisit(Super* n) { unimplementedVisitor("endVisit(Super)"); }

        virtual   bool visit(Interfaces* n) { unimplementedVisitor("visit(Interfaces)"); return true; }
        virtual  void endVisit(Interfaces* n) { unimplementedVisitor("endVisit(Interfaces)"); }

        virtual   bool visit(InterfaceTypeList* n) { unimplementedVisitor("visit(InterfaceTypeList)"); return true; }
        virtual  void endVisit(InterfaceTypeList* n) { unimplementedVisitor("endVisit(InterfaceTypeList)"); }

        virtual   bool visit(ClassBody* n) { unimplementedVisitor("visit(ClassBody)"); return true; }
        virtual  void endVisit(ClassBody* n) { unimplementedVisitor("endVisit(ClassBody)"); }

        virtual   bool visit(ClassBodyDeclarations* n) { unimplementedVisitor("visit(ClassBodyDeclarations)"); return true; }
        virtual  void endVisit(ClassBodyDeclarations* n) { unimplementedVisitor("endVisit(ClassBodyDeclarations)"); }

        virtual   bool visit(ClassMemberDeclaration* n) { unimplementedVisitor("visit(ClassMemberDeclaration)"); return true; }
        virtual  void endVisit(ClassMemberDeclaration* n) { unimplementedVisitor("endVisit(ClassMemberDeclaration)"); }

        virtual   bool visit(FieldDeclaration* n) { unimplementedVisitor("visit(FieldDeclaration)"); return true; }
        virtual  void endVisit(FieldDeclaration* n) { unimplementedVisitor("endVisit(FieldDeclaration)"); }

        virtual   bool visit(VariableDeclarators* n) { unimplementedVisitor("visit(VariableDeclarators)"); return true; }
        virtual  void endVisit(VariableDeclarators* n) { unimplementedVisitor("endVisit(VariableDeclarators)"); }

        virtual   bool visit(VariableDeclarator* n) { unimplementedVisitor("visit(VariableDeclarator)"); return true; }
        virtual  void endVisit(VariableDeclarator* n) { unimplementedVisitor("endVisit(VariableDeclarator)"); }

        virtual   bool visit(VariableDeclaratorId* n) { unimplementedVisitor("visit(VariableDeclaratorId)"); return true; }
        virtual  void endVisit(VariableDeclaratorId* n) { unimplementedVisitor("endVisit(VariableDeclaratorId)"); }

        virtual   bool visit(FieldModifiers* n) { unimplementedVisitor("visit(FieldModifiers)"); return true; }
        virtual  void endVisit(FieldModifiers* n) { unimplementedVisitor("endVisit(FieldModifiers)"); }

        virtual   bool visit(MethodDeclaration* n) { unimplementedVisitor("visit(MethodDeclaration)"); return true; }
        virtual  void endVisit(MethodDeclaration* n) { unimplementedVisitor("endVisit(MethodDeclaration)"); }

        virtual   bool visit(MethodHeader* n) { unimplementedVisitor("visit(MethodHeader)"); return true; }
        virtual  void endVisit(MethodHeader* n) { unimplementedVisitor("endVisit(MethodHeader)"); }

        virtual   bool visit(ResultType* n) { unimplementedVisitor("visit(ResultType)"); return true; }
        virtual  void endVisit(ResultType* n) { unimplementedVisitor("endVisit(ResultType)"); }

        virtual   bool visit(FormalParameterList* n) { unimplementedVisitor("visit(FormalParameterList)"); return true; }
        virtual  void endVisit(FormalParameterList* n) { unimplementedVisitor("endVisit(FormalParameterList)"); }

        virtual   bool visit(FormalParameters* n) { unimplementedVisitor("visit(FormalParameters)"); return true; }
        virtual  void endVisit(FormalParameters* n) { unimplementedVisitor("endVisit(FormalParameters)"); }

        virtual   bool visit(FormalParameter* n) { unimplementedVisitor("visit(FormalParameter)"); return true; }
        virtual  void endVisit(FormalParameter* n) { unimplementedVisitor("endVisit(FormalParameter)"); }

        virtual   bool visit(VariableModifiers* n) { unimplementedVisitor("visit(VariableModifiers)"); return true; }
        virtual  void endVisit(VariableModifiers* n) { unimplementedVisitor("endVisit(VariableModifiers)"); }

        virtual   bool visit(VariableModifier* n) { unimplementedVisitor("visit(VariableModifier)"); return true; }
        virtual  void endVisit(VariableModifier* n) { unimplementedVisitor("endVisit(VariableModifier)"); }

        virtual   bool visit(LastFormalParameter* n) { unimplementedVisitor("visit(LastFormalParameter)"); return true; }
        virtual  void endVisit(LastFormalParameter* n) { unimplementedVisitor("endVisit(LastFormalParameter)"); }

        virtual   bool visit(MethodModifiers* n) { unimplementedVisitor("visit(MethodModifiers)"); return true; }
        virtual  void endVisit(MethodModifiers* n) { unimplementedVisitor("endVisit(MethodModifiers)"); }

        virtual   bool visit(Throws* n) { unimplementedVisitor("visit(Throws)"); return true; }
        virtual  void endVisit(Throws* n) { unimplementedVisitor("endVisit(Throws)"); }

        virtual   bool visit(ExceptionTypeList* n) { unimplementedVisitor("visit(ExceptionTypeList)"); return true; }
        virtual  void endVisit(ExceptionTypeList* n) { unimplementedVisitor("endVisit(ExceptionTypeList)"); }

        virtual   bool visit(MethodBody* n) { unimplementedVisitor("visit(MethodBody)"); return true; }
        virtual  void endVisit(MethodBody* n) { unimplementedVisitor("endVisit(MethodBody)"); }

        virtual   bool visit(StaticInitializer* n) { unimplementedVisitor("visit(StaticInitializer)"); return true; }
        virtual  void endVisit(StaticInitializer* n) { unimplementedVisitor("endVisit(StaticInitializer)"); }

        virtual   bool visit(ConstructorDeclaration* n) { unimplementedVisitor("visit(ConstructorDeclaration)"); return true; }
        virtual  void endVisit(ConstructorDeclaration* n) { unimplementedVisitor("endVisit(ConstructorDeclaration)"); }

        virtual   bool visit(ConstructorDeclarator* n) { unimplementedVisitor("visit(ConstructorDeclarator)"); return true; }
        virtual  void endVisit(ConstructorDeclarator* n) { unimplementedVisitor("endVisit(ConstructorDeclarator)"); }

        virtual   bool visit(ConstructorModifiers* n) { unimplementedVisitor("visit(ConstructorModifiers)"); return true; }
        virtual  void endVisit(ConstructorModifiers* n) { unimplementedVisitor("endVisit(ConstructorModifiers)"); }

        virtual   bool visit(ConstructorBody* n) { unimplementedVisitor("visit(ConstructorBody)"); return true; }
        virtual  void endVisit(ConstructorBody* n) { unimplementedVisitor("endVisit(ConstructorBody)"); }

        virtual   bool visit(EnumDeclaration* n) { unimplementedVisitor("visit(EnumDeclaration)"); return true; }
        virtual  void endVisit(EnumDeclaration* n) { unimplementedVisitor("endVisit(EnumDeclaration)"); }

        virtual   bool visit(EnumBody* n) { unimplementedVisitor("visit(EnumBody)"); return true; }
        virtual  void endVisit(EnumBody* n) { unimplementedVisitor("endVisit(EnumBody)"); }

        virtual   bool visit(EnumConstants* n) { unimplementedVisitor("visit(EnumConstants)"); return true; }
        virtual  void endVisit(EnumConstants* n) { unimplementedVisitor("endVisit(EnumConstants)"); }

        virtual   bool visit(EnumConstant* n) { unimplementedVisitor("visit(EnumConstant)"); return true; }
        virtual  void endVisit(EnumConstant* n) { unimplementedVisitor("endVisit(EnumConstant)"); }

        virtual   bool visit(Arguments* n) { unimplementedVisitor("visit(Arguments)"); return true; }
        virtual  void endVisit(Arguments* n) { unimplementedVisitor("endVisit(Arguments)"); }

        virtual   bool visit(EnumBodyDeclarations* n) { unimplementedVisitor("visit(EnumBodyDeclarations)"); return true; }
        virtual  void endVisit(EnumBodyDeclarations* n) { unimplementedVisitor("endVisit(EnumBodyDeclarations)"); }

        virtual   bool visit(NormalInterfaceDeclaration* n) { unimplementedVisitor("visit(NormalInterfaceDeclaration)"); return true; }
        virtual  void endVisit(NormalInterfaceDeclaration* n) { unimplementedVisitor("endVisit(NormalInterfaceDeclaration)"); }

        virtual   bool visit(InterfaceModifiers* n) { unimplementedVisitor("visit(InterfaceModifiers)"); return true; }
        virtual  void endVisit(InterfaceModifiers* n) { unimplementedVisitor("endVisit(InterfaceModifiers)"); }

        virtual   bool visit(InterfaceBody* n) { unimplementedVisitor("visit(InterfaceBody)"); return true; }
        virtual  void endVisit(InterfaceBody* n) { unimplementedVisitor("endVisit(InterfaceBody)"); }

        virtual   bool visit(InterfaceMemberDeclarations* n) { unimplementedVisitor("visit(InterfaceMemberDeclarations)"); return true; }
        virtual  void endVisit(InterfaceMemberDeclarations* n) { unimplementedVisitor("endVisit(InterfaceMemberDeclarations)"); }

        virtual   bool visit(InterfaceMemberDeclaration* n) { unimplementedVisitor("visit(InterfaceMemberDeclaration)"); return true; }
        virtual  void endVisit(InterfaceMemberDeclaration* n) { unimplementedVisitor("endVisit(InterfaceMemberDeclaration)"); }

        virtual   bool visit(ConstantDeclaration* n) { unimplementedVisitor("visit(ConstantDeclaration)"); return true; }
        virtual  void endVisit(ConstantDeclaration* n) { unimplementedVisitor("endVisit(ConstantDeclaration)"); }

        virtual   bool visit(ConstantModifiers* n) { unimplementedVisitor("visit(ConstantModifiers)"); return true; }
        virtual  void endVisit(ConstantModifiers* n) { unimplementedVisitor("endVisit(ConstantModifiers)"); }

        virtual   bool visit(AbstractMethodDeclaration* n) { unimplementedVisitor("visit(AbstractMethodDeclaration)"); return true; }
        virtual  void endVisit(AbstractMethodDeclaration* n) { unimplementedVisitor("endVisit(AbstractMethodDeclaration)"); }

        virtual   bool visit(AbstractMethodModifiers* n) { unimplementedVisitor("visit(AbstractMethodModifiers)"); return true; }
        virtual  void endVisit(AbstractMethodModifiers* n) { unimplementedVisitor("endVisit(AbstractMethodModifiers)"); }

        virtual   bool visit(AnnotationTypeDeclaration* n) { unimplementedVisitor("visit(AnnotationTypeDeclaration)"); return true; }
        virtual  void endVisit(AnnotationTypeDeclaration* n) { unimplementedVisitor("endVisit(AnnotationTypeDeclaration)"); }

        virtual   bool visit(AnnotationTypeBody* n) { unimplementedVisitor("visit(AnnotationTypeBody)"); return true; }
        virtual  void endVisit(AnnotationTypeBody* n) { unimplementedVisitor("endVisit(AnnotationTypeBody)"); }

        virtual   bool visit(AnnotationTypeElementDeclarations* n) { unimplementedVisitor("visit(AnnotationTypeElementDeclarations)"); return true; }
        virtual  void endVisit(AnnotationTypeElementDeclarations* n) { unimplementedVisitor("endVisit(AnnotationTypeElementDeclarations)"); }

        virtual   bool visit(DefaultValue* n) { unimplementedVisitor("visit(DefaultValue)"); return true; }
        virtual  void endVisit(DefaultValue* n) { unimplementedVisitor("endVisit(DefaultValue)"); }

        virtual   bool visit(Annotations* n) { unimplementedVisitor("visit(Annotations)"); return true; }
        virtual  void endVisit(Annotations* n) { unimplementedVisitor("endVisit(Annotations)"); }

        virtual   bool visit(NormalAnnotation* n) { unimplementedVisitor("visit(NormalAnnotation)"); return true; }
        virtual  void endVisit(NormalAnnotation* n) { unimplementedVisitor("endVisit(NormalAnnotation)"); }

        virtual   bool visit(ElementValuePairs* n) { unimplementedVisitor("visit(ElementValuePairs)"); return true; }
        virtual  void endVisit(ElementValuePairs* n) { unimplementedVisitor("endVisit(ElementValuePairs)"); }

        virtual   bool visit(ElementValuePair* n) { unimplementedVisitor("visit(ElementValuePair)"); return true; }
        virtual  void endVisit(ElementValuePair* n) { unimplementedVisitor("endVisit(ElementValuePair)"); }

        virtual   bool visit(ElementValueArrayInitializer* n) { unimplementedVisitor("visit(ElementValueArrayInitializer)"); return true; }
        virtual  void endVisit(ElementValueArrayInitializer* n) { unimplementedVisitor("endVisit(ElementValueArrayInitializer)"); }

        virtual   bool visit(ElementValues* n) { unimplementedVisitor("visit(ElementValues)"); return true; }
        virtual  void endVisit(ElementValues* n) { unimplementedVisitor("endVisit(ElementValues)"); }

        virtual   bool visit(MarkerAnnotation* n) { unimplementedVisitor("visit(MarkerAnnotation)"); return true; }
        virtual  void endVisit(MarkerAnnotation* n) { unimplementedVisitor("endVisit(MarkerAnnotation)"); }

        virtual   bool visit(SingleElementAnnotation* n) { unimplementedVisitor("visit(SingleElementAnnotation)"); return true; }
        virtual  void endVisit(SingleElementAnnotation* n) { unimplementedVisitor("endVisit(SingleElementAnnotation)"); }

        virtual   bool visit(ArrayInitializer* n) { unimplementedVisitor("visit(ArrayInitializer)"); return true; }
        virtual  void endVisit(ArrayInitializer* n) { unimplementedVisitor("endVisit(ArrayInitializer)"); }

        virtual   bool visit(VariableInitializers* n) { unimplementedVisitor("visit(VariableInitializers)"); return true; }
        virtual  void endVisit(VariableInitializers* n) { unimplementedVisitor("endVisit(VariableInitializers)"); }

        virtual   bool visit(Block* n) { unimplementedVisitor("visit(Block)"); return true; }
        virtual  void endVisit(Block* n) { unimplementedVisitor("endVisit(Block)"); }

        virtual   bool visit(BlockStatements* n) { unimplementedVisitor("visit(BlockStatements)"); return true; }
        virtual  void endVisit(BlockStatements* n) { unimplementedVisitor("endVisit(BlockStatements)"); }

        virtual   bool visit(LocalVariableDeclarationStatement* n) { unimplementedVisitor("visit(LocalVariableDeclarationStatement)"); return true; }
        virtual  void endVisit(LocalVariableDeclarationStatement* n) { unimplementedVisitor("endVisit(LocalVariableDeclarationStatement)"); }

        virtual   bool visit(LocalVariableDeclaration* n) { unimplementedVisitor("visit(LocalVariableDeclaration)"); return true; }
        virtual  void endVisit(LocalVariableDeclaration* n) { unimplementedVisitor("endVisit(LocalVariableDeclaration)"); }

        virtual   bool visit(IfThenStatement* n) { unimplementedVisitor("visit(IfThenStatement)"); return true; }
        virtual  void endVisit(IfThenStatement* n) { unimplementedVisitor("endVisit(IfThenStatement)"); }

        virtual   bool visit(IfThenElseStatement* n) { unimplementedVisitor("visit(IfThenElseStatement)"); return true; }
        virtual  void endVisit(IfThenElseStatement* n) { unimplementedVisitor("endVisit(IfThenElseStatement)"); }

        virtual   bool visit(IfThenElseStatementNoShortIf* n) { unimplementedVisitor("visit(IfThenElseStatementNoShortIf)"); return true; }
        virtual  void endVisit(IfThenElseStatementNoShortIf* n) { unimplementedVisitor("endVisit(IfThenElseStatementNoShortIf)"); }

        virtual   bool visit(EmptyStatement* n) { unimplementedVisitor("visit(EmptyStatement)"); return true; }
        virtual  void endVisit(EmptyStatement* n) { unimplementedVisitor("endVisit(EmptyStatement)"); }

        virtual   bool visit(LabeledStatement* n) { unimplementedVisitor("visit(LabeledStatement)"); return true; }
        virtual  void endVisit(LabeledStatement* n) { unimplementedVisitor("endVisit(LabeledStatement)"); }

        virtual   bool visit(LabeledStatementNoShortIf* n) { unimplementedVisitor("visit(LabeledStatementNoShortIf)"); return true; }
        virtual  void endVisit(LabeledStatementNoShortIf* n) { unimplementedVisitor("endVisit(LabeledStatementNoShortIf)"); }

        virtual   bool visit(ExpressionStatement* n) { unimplementedVisitor("visit(ExpressionStatement)"); return true; }
        virtual  void endVisit(ExpressionStatement* n) { unimplementedVisitor("endVisit(ExpressionStatement)"); }

        virtual   bool visit(SwitchStatement* n) { unimplementedVisitor("visit(SwitchStatement)"); return true; }
        virtual  void endVisit(SwitchStatement* n) { unimplementedVisitor("endVisit(SwitchStatement)"); }

        virtual   bool visit(SwitchBlock* n) { unimplementedVisitor("visit(SwitchBlock)"); return true; }
        virtual  void endVisit(SwitchBlock* n) { unimplementedVisitor("endVisit(SwitchBlock)"); }

        virtual   bool visit(SwitchBlockStatementGroups* n) { unimplementedVisitor("visit(SwitchBlockStatementGroups)"); return true; }
        virtual  void endVisit(SwitchBlockStatementGroups* n) { unimplementedVisitor("endVisit(SwitchBlockStatementGroups)"); }

        virtual   bool visit(SwitchBlockStatementGroup* n) { unimplementedVisitor("visit(SwitchBlockStatementGroup)"); return true; }
        virtual  void endVisit(SwitchBlockStatementGroup* n) { unimplementedVisitor("endVisit(SwitchBlockStatementGroup)"); }

        virtual   bool visit(SwitchLabels* n) { unimplementedVisitor("visit(SwitchLabels)"); return true; }
        virtual  void endVisit(SwitchLabels* n) { unimplementedVisitor("endVisit(SwitchLabels)"); }

        virtual   bool visit(WhileStatement* n) { unimplementedVisitor("visit(WhileStatement)"); return true; }
        virtual  void endVisit(WhileStatement* n) { unimplementedVisitor("endVisit(WhileStatement)"); }

        virtual   bool visit(WhileStatementNoShortIf* n) { unimplementedVisitor("visit(WhileStatementNoShortIf)"); return true; }
        virtual  void endVisit(WhileStatementNoShortIf* n) { unimplementedVisitor("endVisit(WhileStatementNoShortIf)"); }

        virtual   bool visit(DoStatement* n) { unimplementedVisitor("visit(DoStatement)"); return true; }
        virtual  void endVisit(DoStatement* n) { unimplementedVisitor("endVisit(DoStatement)"); }

        virtual   bool visit(BasicForStatement* n) { unimplementedVisitor("visit(BasicForStatement)"); return true; }
        virtual  void endVisit(BasicForStatement* n) { unimplementedVisitor("endVisit(BasicForStatement)"); }

        virtual   bool visit(ForStatementNoShortIf* n) { unimplementedVisitor("visit(ForStatementNoShortIf)"); return true; }
        virtual  void endVisit(ForStatementNoShortIf* n) { unimplementedVisitor("endVisit(ForStatementNoShortIf)"); }

        virtual   bool visit(StatementExpressionList* n) { unimplementedVisitor("visit(StatementExpressionList)"); return true; }
        virtual  void endVisit(StatementExpressionList* n) { unimplementedVisitor("endVisit(StatementExpressionList)"); }

        virtual   bool visit(EnhancedForStatement* n) { unimplementedVisitor("visit(EnhancedForStatement)"); return true; }
        virtual  void endVisit(EnhancedForStatement* n) { unimplementedVisitor("endVisit(EnhancedForStatement)"); }

        virtual   bool visit(BreakStatement* n) { unimplementedVisitor("visit(BreakStatement)"); return true; }
        virtual  void endVisit(BreakStatement* n) { unimplementedVisitor("endVisit(BreakStatement)"); }

        virtual   bool visit(ContinueStatement* n) { unimplementedVisitor("visit(ContinueStatement)"); return true; }
        virtual  void endVisit(ContinueStatement* n) { unimplementedVisitor("endVisit(ContinueStatement)"); }

        virtual   bool visit(ReturnStatement* n) { unimplementedVisitor("visit(ReturnStatement)"); return true; }
        virtual  void endVisit(ReturnStatement* n) { unimplementedVisitor("endVisit(ReturnStatement)"); }

        virtual   bool visit(ThrowStatement* n) { unimplementedVisitor("visit(ThrowStatement)"); return true; }
        virtual  void endVisit(ThrowStatement* n) { unimplementedVisitor("endVisit(ThrowStatement)"); }

        virtual   bool visit(SynchronizedStatement* n) { unimplementedVisitor("visit(SynchronizedStatement)"); return true; }
        virtual  void endVisit(SynchronizedStatement* n) { unimplementedVisitor("endVisit(SynchronizedStatement)"); }

        virtual   bool visit(Catches* n) { unimplementedVisitor("visit(Catches)"); return true; }
        virtual  void endVisit(Catches* n) { unimplementedVisitor("endVisit(Catches)"); }

        virtual   bool visit(CatchClause* n) { unimplementedVisitor("visit(CatchClause)"); return true; }
        virtual  void endVisit(CatchClause* n) { unimplementedVisitor("endVisit(CatchClause)"); }

        virtual   bool visit(Finally* n) { unimplementedVisitor("visit(Finally)"); return true; }
        virtual  void endVisit(Finally* n) { unimplementedVisitor("endVisit(Finally)"); }

        virtual   bool visit(ArgumentList* n) { unimplementedVisitor("visit(ArgumentList)"); return true; }
        virtual  void endVisit(ArgumentList* n) { unimplementedVisitor("endVisit(ArgumentList)"); }

        virtual   bool visit(DimExprs* n) { unimplementedVisitor("visit(DimExprs)"); return true; }
        virtual  void endVisit(DimExprs* n) { unimplementedVisitor("endVisit(DimExprs)"); }

        virtual   bool visit(DimExpr* n) { unimplementedVisitor("visit(DimExpr)"); return true; }
        virtual  void endVisit(DimExpr* n) { unimplementedVisitor("endVisit(DimExpr)"); }

        virtual   bool visit(PostIncrementExpression* n) { unimplementedVisitor("visit(PostIncrementExpression)"); return true; }
        virtual  void endVisit(PostIncrementExpression* n) { unimplementedVisitor("endVisit(PostIncrementExpression)"); }

        virtual   bool visit(PostDecrementExpression* n) { unimplementedVisitor("visit(PostDecrementExpression)"); return true; }
        virtual  void endVisit(PostDecrementExpression* n) { unimplementedVisitor("endVisit(PostDecrementExpression)"); }

        virtual   bool visit(PreIncrementExpression* n) { unimplementedVisitor("visit(PreIncrementExpression)"); return true; }
        virtual  void endVisit(PreIncrementExpression* n) { unimplementedVisitor("endVisit(PreIncrementExpression)"); }

        virtual   bool visit(PreDecrementExpression* n) { unimplementedVisitor("visit(PreDecrementExpression)"); return true; }
        virtual  void endVisit(PreDecrementExpression* n) { unimplementedVisitor("endVisit(PreDecrementExpression)"); }

        virtual   bool visit(AndExpression* n) { unimplementedVisitor("visit(AndExpression)"); return true; }
        virtual  void endVisit(AndExpression* n) { unimplementedVisitor("endVisit(AndExpression)"); }

        virtual   bool visit(ExclusiveOrExpression* n) { unimplementedVisitor("visit(ExclusiveOrExpression)"); return true; }
        virtual  void endVisit(ExclusiveOrExpression* n) { unimplementedVisitor("endVisit(ExclusiveOrExpression)"); }

        virtual   bool visit(InclusiveOrExpression* n) { unimplementedVisitor("visit(InclusiveOrExpression)"); return true; }
        virtual  void endVisit(InclusiveOrExpression* n) { unimplementedVisitor("endVisit(InclusiveOrExpression)"); }

        virtual   bool visit(ConditionalAndExpression* n) { unimplementedVisitor("visit(ConditionalAndExpression)"); return true; }
        virtual  void endVisit(ConditionalAndExpression* n) { unimplementedVisitor("endVisit(ConditionalAndExpression)"); }

        virtual   bool visit(ConditionalOrExpression* n) { unimplementedVisitor("visit(ConditionalOrExpression)"); return true; }
        virtual  void endVisit(ConditionalOrExpression* n) { unimplementedVisitor("endVisit(ConditionalOrExpression)"); }

        virtual   bool visit(ConditionalExpression* n) { unimplementedVisitor("visit(ConditionalExpression)"); return true; }
        virtual  void endVisit(ConditionalExpression* n) { unimplementedVisitor("endVisit(ConditionalExpression)"); }

        virtual   bool visit(Assignment* n) { unimplementedVisitor("visit(Assignment)"); return true; }
        virtual  void endVisit(Assignment* n) { unimplementedVisitor("endVisit(Assignment)"); }

        virtual   bool visit(Commaopt* n) { unimplementedVisitor("visit(Commaopt)"); return true; }
        virtual  void endVisit(Commaopt* n) { unimplementedVisitor("endVisit(Commaopt)"); }

        virtual   bool visit(Ellipsisopt* n) { unimplementedVisitor("visit(Ellipsisopt)"); return true; }
        virtual  void endVisit(Ellipsisopt* n) { unimplementedVisitor("endVisit(Ellipsisopt)"); }

        virtual   bool visit(LPGUserAction0* n) { unimplementedVisitor("visit(LPGUserAction0)"); return true; }
        virtual  void endVisit(LPGUserAction0* n) { unimplementedVisitor("endVisit(LPGUserAction0)"); }

        virtual   bool visit(LPGUserAction1* n) { unimplementedVisitor("visit(LPGUserAction1)"); return true; }
        virtual  void endVisit(LPGUserAction1* n) { unimplementedVisitor("endVisit(LPGUserAction1)"); }

        virtual   bool visit(LPGUserAction2* n) { unimplementedVisitor("visit(LPGUserAction2)"); return true; }
        virtual  void endVisit(LPGUserAction2* n) { unimplementedVisitor("endVisit(LPGUserAction2)"); }

        virtual   bool visit(LPGUserAction3* n) { unimplementedVisitor("visit(LPGUserAction3)"); return true; }
        virtual  void endVisit(LPGUserAction3* n) { unimplementedVisitor("endVisit(LPGUserAction3)"); }

        virtual   bool visit(LPGUserAction4* n) { unimplementedVisitor("visit(LPGUserAction4)"); return true; }
        virtual  void endVisit(LPGUserAction4* n) { unimplementedVisitor("endVisit(LPGUserAction4)"); }

        virtual   bool visit(IntegralType0* n) { unimplementedVisitor("visit(IntegralType0)"); return true; }
        virtual  void endVisit(IntegralType0* n) { unimplementedVisitor("endVisit(IntegralType0)"); }

        virtual   bool visit(IntegralType1* n) { unimplementedVisitor("visit(IntegralType1)"); return true; }
        virtual  void endVisit(IntegralType1* n) { unimplementedVisitor("endVisit(IntegralType1)"); }

        virtual   bool visit(IntegralType2* n) { unimplementedVisitor("visit(IntegralType2)"); return true; }
        virtual  void endVisit(IntegralType2* n) { unimplementedVisitor("endVisit(IntegralType2)"); }

        virtual   bool visit(IntegralType3* n) { unimplementedVisitor("visit(IntegralType3)"); return true; }
        virtual  void endVisit(IntegralType3* n) { unimplementedVisitor("endVisit(IntegralType3)"); }

        virtual   bool visit(IntegralType4* n) { unimplementedVisitor("visit(IntegralType4)"); return true; }
        virtual  void endVisit(IntegralType4* n) { unimplementedVisitor("endVisit(IntegralType4)"); }

        virtual   bool visit(FloatingPointType0* n) { unimplementedVisitor("visit(FloatingPointType0)"); return true; }
        virtual  void endVisit(FloatingPointType0* n) { unimplementedVisitor("endVisit(FloatingPointType0)"); }

        virtual   bool visit(FloatingPointType1* n) { unimplementedVisitor("visit(FloatingPointType1)"); return true; }
        virtual  void endVisit(FloatingPointType1* n) { unimplementedVisitor("endVisit(FloatingPointType1)"); }

        virtual   bool visit(WildcardBounds0* n) { unimplementedVisitor("visit(WildcardBounds0)"); return true; }
        virtual  void endVisit(WildcardBounds0* n) { unimplementedVisitor("endVisit(WildcardBounds0)"); }

        virtual   bool visit(WildcardBounds1* n) { unimplementedVisitor("visit(WildcardBounds1)"); return true; }
        virtual  void endVisit(WildcardBounds1* n) { unimplementedVisitor("endVisit(WildcardBounds1)"); }

        virtual   bool visit(ClassModifier0* n) { unimplementedVisitor("visit(ClassModifier0)"); return true; }
        virtual  void endVisit(ClassModifier0* n) { unimplementedVisitor("endVisit(ClassModifier0)"); }

        virtual   bool visit(ClassModifier1* n) { unimplementedVisitor("visit(ClassModifier1)"); return true; }
        virtual  void endVisit(ClassModifier1* n) { unimplementedVisitor("endVisit(ClassModifier1)"); }

        virtual   bool visit(ClassModifier2* n) { unimplementedVisitor("visit(ClassModifier2)"); return true; }
        virtual  void endVisit(ClassModifier2* n) { unimplementedVisitor("endVisit(ClassModifier2)"); }

        virtual   bool visit(ClassModifier3* n) { unimplementedVisitor("visit(ClassModifier3)"); return true; }
        virtual  void endVisit(ClassModifier3* n) { unimplementedVisitor("endVisit(ClassModifier3)"); }

        virtual   bool visit(ClassModifier4* n) { unimplementedVisitor("visit(ClassModifier4)"); return true; }
        virtual  void endVisit(ClassModifier4* n) { unimplementedVisitor("endVisit(ClassModifier4)"); }

        virtual   bool visit(ClassModifier5* n) { unimplementedVisitor("visit(ClassModifier5)"); return true; }
        virtual  void endVisit(ClassModifier5* n) { unimplementedVisitor("endVisit(ClassModifier5)"); }

        virtual   bool visit(ClassModifier6* n) { unimplementedVisitor("visit(ClassModifier6)"); return true; }
        virtual  void endVisit(ClassModifier6* n) { unimplementedVisitor("endVisit(ClassModifier6)"); }

        virtual   bool visit(FieldModifier0* n) { unimplementedVisitor("visit(FieldModifier0)"); return true; }
        virtual  void endVisit(FieldModifier0* n) { unimplementedVisitor("endVisit(FieldModifier0)"); }

        virtual   bool visit(FieldModifier1* n) { unimplementedVisitor("visit(FieldModifier1)"); return true; }
        virtual  void endVisit(FieldModifier1* n) { unimplementedVisitor("endVisit(FieldModifier1)"); }

        virtual   bool visit(FieldModifier2* n) { unimplementedVisitor("visit(FieldModifier2)"); return true; }
        virtual  void endVisit(FieldModifier2* n) { unimplementedVisitor("endVisit(FieldModifier2)"); }

        virtual   bool visit(FieldModifier3* n) { unimplementedVisitor("visit(FieldModifier3)"); return true; }
        virtual  void endVisit(FieldModifier3* n) { unimplementedVisitor("endVisit(FieldModifier3)"); }

        virtual   bool visit(FieldModifier4* n) { unimplementedVisitor("visit(FieldModifier4)"); return true; }
        virtual  void endVisit(FieldModifier4* n) { unimplementedVisitor("endVisit(FieldModifier4)"); }

        virtual   bool visit(FieldModifier5* n) { unimplementedVisitor("visit(FieldModifier5)"); return true; }
        virtual  void endVisit(FieldModifier5* n) { unimplementedVisitor("endVisit(FieldModifier5)"); }

        virtual   bool visit(FieldModifier6* n) { unimplementedVisitor("visit(FieldModifier6)"); return true; }
        virtual  void endVisit(FieldModifier6* n) { unimplementedVisitor("endVisit(FieldModifier6)"); }

        virtual   bool visit(MethodDeclarator0* n) { unimplementedVisitor("visit(MethodDeclarator0)"); return true; }
        virtual  void endVisit(MethodDeclarator0* n) { unimplementedVisitor("endVisit(MethodDeclarator0)"); }

        virtual   bool visit(MethodDeclarator1* n) { unimplementedVisitor("visit(MethodDeclarator1)"); return true; }
        virtual  void endVisit(MethodDeclarator1* n) { unimplementedVisitor("endVisit(MethodDeclarator1)"); }

        virtual   bool visit(MethodModifier0* n) { unimplementedVisitor("visit(MethodModifier0)"); return true; }
        virtual  void endVisit(MethodModifier0* n) { unimplementedVisitor("endVisit(MethodModifier0)"); }

        virtual   bool visit(MethodModifier1* n) { unimplementedVisitor("visit(MethodModifier1)"); return true; }
        virtual  void endVisit(MethodModifier1* n) { unimplementedVisitor("endVisit(MethodModifier1)"); }

        virtual   bool visit(MethodModifier2* n) { unimplementedVisitor("visit(MethodModifier2)"); return true; }
        virtual  void endVisit(MethodModifier2* n) { unimplementedVisitor("endVisit(MethodModifier2)"); }

        virtual   bool visit(MethodModifier3* n) { unimplementedVisitor("visit(MethodModifier3)"); return true; }
        virtual  void endVisit(MethodModifier3* n) { unimplementedVisitor("endVisit(MethodModifier3)"); }

        virtual   bool visit(MethodModifier4* n) { unimplementedVisitor("visit(MethodModifier4)"); return true; }
        virtual  void endVisit(MethodModifier4* n) { unimplementedVisitor("endVisit(MethodModifier4)"); }

        virtual   bool visit(MethodModifier5* n) { unimplementedVisitor("visit(MethodModifier5)"); return true; }
        virtual  void endVisit(MethodModifier5* n) { unimplementedVisitor("endVisit(MethodModifier5)"); }

        virtual   bool visit(MethodModifier6* n) { unimplementedVisitor("visit(MethodModifier6)"); return true; }
        virtual  void endVisit(MethodModifier6* n) { unimplementedVisitor("endVisit(MethodModifier6)"); }

        virtual   bool visit(MethodModifier7* n) { unimplementedVisitor("visit(MethodModifier7)"); return true; }
        virtual  void endVisit(MethodModifier7* n) { unimplementedVisitor("endVisit(MethodModifier7)"); }

        virtual   bool visit(MethodModifier8* n) { unimplementedVisitor("visit(MethodModifier8)"); return true; }
        virtual  void endVisit(MethodModifier8* n) { unimplementedVisitor("endVisit(MethodModifier8)"); }

        virtual   bool visit(ConstructorModifier0* n) { unimplementedVisitor("visit(ConstructorModifier0)"); return true; }
        virtual  void endVisit(ConstructorModifier0* n) { unimplementedVisitor("endVisit(ConstructorModifier0)"); }

        virtual   bool visit(ConstructorModifier1* n) { unimplementedVisitor("visit(ConstructorModifier1)"); return true; }
        virtual  void endVisit(ConstructorModifier1* n) { unimplementedVisitor("endVisit(ConstructorModifier1)"); }

        virtual   bool visit(ConstructorModifier2* n) { unimplementedVisitor("visit(ConstructorModifier2)"); return true; }
        virtual  void endVisit(ConstructorModifier2* n) { unimplementedVisitor("endVisit(ConstructorModifier2)"); }

        virtual   bool visit(ExplicitConstructorInvocation0* n) { unimplementedVisitor("visit(ExplicitConstructorInvocation0)"); return true; }
        virtual  void endVisit(ExplicitConstructorInvocation0* n) { unimplementedVisitor("endVisit(ExplicitConstructorInvocation0)"); }

        virtual   bool visit(ExplicitConstructorInvocation1* n) { unimplementedVisitor("visit(ExplicitConstructorInvocation1)"); return true; }
        virtual  void endVisit(ExplicitConstructorInvocation1* n) { unimplementedVisitor("endVisit(ExplicitConstructorInvocation1)"); }

        virtual   bool visit(ExplicitConstructorInvocation2* n) { unimplementedVisitor("visit(ExplicitConstructorInvocation2)"); return true; }
        virtual  void endVisit(ExplicitConstructorInvocation2* n) { unimplementedVisitor("endVisit(ExplicitConstructorInvocation2)"); }

        virtual   bool visit(InterfaceModifier0* n) { unimplementedVisitor("visit(InterfaceModifier0)"); return true; }
        virtual  void endVisit(InterfaceModifier0* n) { unimplementedVisitor("endVisit(InterfaceModifier0)"); }

        virtual   bool visit(InterfaceModifier1* n) { unimplementedVisitor("visit(InterfaceModifier1)"); return true; }
        virtual  void endVisit(InterfaceModifier1* n) { unimplementedVisitor("endVisit(InterfaceModifier1)"); }

        virtual   bool visit(InterfaceModifier2* n) { unimplementedVisitor("visit(InterfaceModifier2)"); return true; }
        virtual  void endVisit(InterfaceModifier2* n) { unimplementedVisitor("endVisit(InterfaceModifier2)"); }

        virtual   bool visit(InterfaceModifier3* n) { unimplementedVisitor("visit(InterfaceModifier3)"); return true; }
        virtual  void endVisit(InterfaceModifier3* n) { unimplementedVisitor("endVisit(InterfaceModifier3)"); }

        virtual   bool visit(InterfaceModifier4* n) { unimplementedVisitor("visit(InterfaceModifier4)"); return true; }
        virtual  void endVisit(InterfaceModifier4* n) { unimplementedVisitor("endVisit(InterfaceModifier4)"); }

        virtual   bool visit(InterfaceModifier5* n) { unimplementedVisitor("visit(InterfaceModifier5)"); return true; }
        virtual  void endVisit(InterfaceModifier5* n) { unimplementedVisitor("endVisit(InterfaceModifier5)"); }

        virtual   bool visit(ExtendsInterfaces0* n) { unimplementedVisitor("visit(ExtendsInterfaces0)"); return true; }
        virtual  void endVisit(ExtendsInterfaces0* n) { unimplementedVisitor("endVisit(ExtendsInterfaces0)"); }

        virtual   bool visit(ExtendsInterfaces1* n) { unimplementedVisitor("visit(ExtendsInterfaces1)"); return true; }
        virtual  void endVisit(ExtendsInterfaces1* n) { unimplementedVisitor("endVisit(ExtendsInterfaces1)"); }

        virtual   bool visit(ConstantModifier0* n) { unimplementedVisitor("visit(ConstantModifier0)"); return true; }
        virtual  void endVisit(ConstantModifier0* n) { unimplementedVisitor("endVisit(ConstantModifier0)"); }

        virtual   bool visit(ConstantModifier1* n) { unimplementedVisitor("visit(ConstantModifier1)"); return true; }
        virtual  void endVisit(ConstantModifier1* n) { unimplementedVisitor("endVisit(ConstantModifier1)"); }

        virtual   bool visit(ConstantModifier2* n) { unimplementedVisitor("visit(ConstantModifier2)"); return true; }
        virtual  void endVisit(ConstantModifier2* n) { unimplementedVisitor("endVisit(ConstantModifier2)"); }

        virtual   bool visit(AbstractMethodModifier0* n) { unimplementedVisitor("visit(AbstractMethodModifier0)"); return true; }
        virtual  void endVisit(AbstractMethodModifier0* n) { unimplementedVisitor("endVisit(AbstractMethodModifier0)"); }

        virtual   bool visit(AbstractMethodModifier1* n) { unimplementedVisitor("visit(AbstractMethodModifier1)"); return true; }
        virtual  void endVisit(AbstractMethodModifier1* n) { unimplementedVisitor("endVisit(AbstractMethodModifier1)"); }

        virtual   bool visit(AnnotationTypeElementDeclaration0* n) { unimplementedVisitor("visit(AnnotationTypeElementDeclaration0)"); return true; }
        virtual  void endVisit(AnnotationTypeElementDeclaration0* n) { unimplementedVisitor("endVisit(AnnotationTypeElementDeclaration0)"); }

        virtual   bool visit(AnnotationTypeElementDeclaration1* n) { unimplementedVisitor("visit(AnnotationTypeElementDeclaration1)"); return true; }
        virtual  void endVisit(AnnotationTypeElementDeclaration1* n) { unimplementedVisitor("endVisit(AnnotationTypeElementDeclaration1)"); }

        virtual   bool visit(AssertStatement0* n) { unimplementedVisitor("visit(AssertStatement0)"); return true; }
        virtual  void endVisit(AssertStatement0* n) { unimplementedVisitor("endVisit(AssertStatement0)"); }

        virtual   bool visit(AssertStatement1* n) { unimplementedVisitor("visit(AssertStatement1)"); return true; }
        virtual  void endVisit(AssertStatement1* n) { unimplementedVisitor("endVisit(AssertStatement1)"); }

        virtual   bool visit(SwitchLabel0* n) { unimplementedVisitor("visit(SwitchLabel0)"); return true; }
        virtual  void endVisit(SwitchLabel0* n) { unimplementedVisitor("endVisit(SwitchLabel0)"); }

        virtual   bool visit(SwitchLabel1* n) { unimplementedVisitor("visit(SwitchLabel1)"); return true; }
        virtual  void endVisit(SwitchLabel1* n) { unimplementedVisitor("endVisit(SwitchLabel1)"); }

        virtual   bool visit(SwitchLabel2* n) { unimplementedVisitor("visit(SwitchLabel2)"); return true; }
        virtual  void endVisit(SwitchLabel2* n) { unimplementedVisitor("endVisit(SwitchLabel2)"); }

        virtual   bool visit(TryStatement0* n) { unimplementedVisitor("visit(TryStatement0)"); return true; }
        virtual  void endVisit(TryStatement0* n) { unimplementedVisitor("endVisit(TryStatement0)"); }

        virtual   bool visit(TryStatement1* n) { unimplementedVisitor("visit(TryStatement1)"); return true; }
        virtual  void endVisit(TryStatement1* n) { unimplementedVisitor("endVisit(TryStatement1)"); }

        virtual   bool visit(PrimaryNoNewArray0* n) { unimplementedVisitor("visit(PrimaryNoNewArray0)"); return true; }
        virtual  void endVisit(PrimaryNoNewArray0* n) { unimplementedVisitor("endVisit(PrimaryNoNewArray0)"); }

        virtual   bool visit(PrimaryNoNewArray1* n) { unimplementedVisitor("visit(PrimaryNoNewArray1)"); return true; }
        virtual  void endVisit(PrimaryNoNewArray1* n) { unimplementedVisitor("endVisit(PrimaryNoNewArray1)"); }

        virtual   bool visit(PrimaryNoNewArray2* n) { unimplementedVisitor("visit(PrimaryNoNewArray2)"); return true; }
        virtual  void endVisit(PrimaryNoNewArray2* n) { unimplementedVisitor("endVisit(PrimaryNoNewArray2)"); }

        virtual   bool visit(PrimaryNoNewArray3* n) { unimplementedVisitor("visit(PrimaryNoNewArray3)"); return true; }
        virtual  void endVisit(PrimaryNoNewArray3* n) { unimplementedVisitor("endVisit(PrimaryNoNewArray3)"); }

        virtual   bool visit(PrimaryNoNewArray4* n) { unimplementedVisitor("visit(PrimaryNoNewArray4)"); return true; }
        virtual  void endVisit(PrimaryNoNewArray4* n) { unimplementedVisitor("endVisit(PrimaryNoNewArray4)"); }

        virtual   bool visit(Literal0* n) { unimplementedVisitor("visit(Literal0)"); return true; }
        virtual  void endVisit(Literal0* n) { unimplementedVisitor("endVisit(Literal0)"); }

        virtual   bool visit(Literal1* n) { unimplementedVisitor("visit(Literal1)"); return true; }
        virtual  void endVisit(Literal1* n) { unimplementedVisitor("endVisit(Literal1)"); }

        virtual   bool visit(Literal2* n) { unimplementedVisitor("visit(Literal2)"); return true; }
        virtual  void endVisit(Literal2* n) { unimplementedVisitor("endVisit(Literal2)"); }

        virtual   bool visit(Literal3* n) { unimplementedVisitor("visit(Literal3)"); return true; }
        virtual  void endVisit(Literal3* n) { unimplementedVisitor("endVisit(Literal3)"); }

        virtual   bool visit(Literal4* n) { unimplementedVisitor("visit(Literal4)"); return true; }
        virtual  void endVisit(Literal4* n) { unimplementedVisitor("endVisit(Literal4)"); }

        virtual   bool visit(Literal5* n) { unimplementedVisitor("visit(Literal5)"); return true; }
        virtual  void endVisit(Literal5* n) { unimplementedVisitor("endVisit(Literal5)"); }

        virtual   bool visit(Literal6* n) { unimplementedVisitor("visit(Literal6)"); return true; }
        virtual  void endVisit(Literal6* n) { unimplementedVisitor("endVisit(Literal6)"); }

        virtual   bool visit(BooleanLiteral0* n) { unimplementedVisitor("visit(BooleanLiteral0)"); return true; }
        virtual  void endVisit(BooleanLiteral0* n) { unimplementedVisitor("endVisit(BooleanLiteral0)"); }

        virtual   bool visit(BooleanLiteral1* n) { unimplementedVisitor("visit(BooleanLiteral1)"); return true; }
        virtual  void endVisit(BooleanLiteral1* n) { unimplementedVisitor("endVisit(BooleanLiteral1)"); }

        virtual   bool visit(ClassInstanceCreationExpression0* n) { unimplementedVisitor("visit(ClassInstanceCreationExpression0)"); return true; }
        virtual  void endVisit(ClassInstanceCreationExpression0* n) { unimplementedVisitor("endVisit(ClassInstanceCreationExpression0)"); }

        virtual   bool visit(ClassInstanceCreationExpression1* n) { unimplementedVisitor("visit(ClassInstanceCreationExpression1)"); return true; }
        virtual  void endVisit(ClassInstanceCreationExpression1* n) { unimplementedVisitor("endVisit(ClassInstanceCreationExpression1)"); }

        virtual   bool visit(ArrayCreationExpression0* n) { unimplementedVisitor("visit(ArrayCreationExpression0)"); return true; }
        virtual  void endVisit(ArrayCreationExpression0* n) { unimplementedVisitor("endVisit(ArrayCreationExpression0)"); }

        virtual   bool visit(ArrayCreationExpression1* n) { unimplementedVisitor("visit(ArrayCreationExpression1)"); return true; }
        virtual  void endVisit(ArrayCreationExpression1* n) { unimplementedVisitor("endVisit(ArrayCreationExpression1)"); }

        virtual   bool visit(ArrayCreationExpression2* n) { unimplementedVisitor("visit(ArrayCreationExpression2)"); return true; }
        virtual  void endVisit(ArrayCreationExpression2* n) { unimplementedVisitor("endVisit(ArrayCreationExpression2)"); }

        virtual   bool visit(ArrayCreationExpression3* n) { unimplementedVisitor("visit(ArrayCreationExpression3)"); return true; }
        virtual  void endVisit(ArrayCreationExpression3* n) { unimplementedVisitor("endVisit(ArrayCreationExpression3)"); }

        virtual   bool visit(Dims0* n) { unimplementedVisitor("visit(Dims0)"); return true; }
        virtual  void endVisit(Dims0* n) { unimplementedVisitor("endVisit(Dims0)"); }

        virtual   bool visit(Dims1* n) { unimplementedVisitor("visit(Dims1)"); return true; }
        virtual  void endVisit(Dims1* n) { unimplementedVisitor("endVisit(Dims1)"); }

        virtual   bool visit(FieldAccess0* n) { unimplementedVisitor("visit(FieldAccess0)"); return true; }
        virtual  void endVisit(FieldAccess0* n) { unimplementedVisitor("endVisit(FieldAccess0)"); }

        virtual   bool visit(FieldAccess1* n) { unimplementedVisitor("visit(FieldAccess1)"); return true; }
        virtual  void endVisit(FieldAccess1* n) { unimplementedVisitor("endVisit(FieldAccess1)"); }

        virtual   bool visit(FieldAccess2* n) { unimplementedVisitor("visit(FieldAccess2)"); return true; }
        virtual  void endVisit(FieldAccess2* n) { unimplementedVisitor("endVisit(FieldAccess2)"); }

        virtual   bool visit(MethodInvocation0* n) { unimplementedVisitor("visit(MethodInvocation0)"); return true; }
        virtual  void endVisit(MethodInvocation0* n) { unimplementedVisitor("endVisit(MethodInvocation0)"); }

        virtual   bool visit(MethodInvocation1* n) { unimplementedVisitor("visit(MethodInvocation1)"); return true; }
        virtual  void endVisit(MethodInvocation1* n) { unimplementedVisitor("endVisit(MethodInvocation1)"); }

        virtual   bool visit(MethodInvocation2* n) { unimplementedVisitor("visit(MethodInvocation2)"); return true; }
        virtual  void endVisit(MethodInvocation2* n) { unimplementedVisitor("endVisit(MethodInvocation2)"); }

        virtual   bool visit(MethodInvocation3* n) { unimplementedVisitor("visit(MethodInvocation3)"); return true; }
        virtual  void endVisit(MethodInvocation3* n) { unimplementedVisitor("endVisit(MethodInvocation3)"); }

        virtual   bool visit(MethodInvocation4* n) { unimplementedVisitor("visit(MethodInvocation4)"); return true; }
        virtual  void endVisit(MethodInvocation4* n) { unimplementedVisitor("endVisit(MethodInvocation4)"); }

        virtual   bool visit(ArrayAccess0* n) { unimplementedVisitor("visit(ArrayAccess0)"); return true; }
        virtual  void endVisit(ArrayAccess0* n) { unimplementedVisitor("endVisit(ArrayAccess0)"); }

        virtual   bool visit(ArrayAccess1* n) { unimplementedVisitor("visit(ArrayAccess1)"); return true; }
        virtual  void endVisit(ArrayAccess1* n) { unimplementedVisitor("endVisit(ArrayAccess1)"); }

        virtual   bool visit(UnaryExpression0* n) { unimplementedVisitor("visit(UnaryExpression0)"); return true; }
        virtual  void endVisit(UnaryExpression0* n) { unimplementedVisitor("endVisit(UnaryExpression0)"); }

        virtual   bool visit(UnaryExpression1* n) { unimplementedVisitor("visit(UnaryExpression1)"); return true; }
        virtual  void endVisit(UnaryExpression1* n) { unimplementedVisitor("endVisit(UnaryExpression1)"); }

        virtual   bool visit(UnaryExpressionNotPlusMinus0* n) { unimplementedVisitor("visit(UnaryExpressionNotPlusMinus0)"); return true; }
        virtual  void endVisit(UnaryExpressionNotPlusMinus0* n) { unimplementedVisitor("endVisit(UnaryExpressionNotPlusMinus0)"); }

        virtual   bool visit(UnaryExpressionNotPlusMinus1* n) { unimplementedVisitor("visit(UnaryExpressionNotPlusMinus1)"); return true; }
        virtual  void endVisit(UnaryExpressionNotPlusMinus1* n) { unimplementedVisitor("endVisit(UnaryExpressionNotPlusMinus1)"); }

        virtual   bool visit(CastExpression0* n) { unimplementedVisitor("visit(CastExpression0)"); return true; }
        virtual  void endVisit(CastExpression0* n) { unimplementedVisitor("endVisit(CastExpression0)"); }

        virtual   bool visit(CastExpression1* n) { unimplementedVisitor("visit(CastExpression1)"); return true; }
        virtual  void endVisit(CastExpression1* n) { unimplementedVisitor("endVisit(CastExpression1)"); }

        virtual   bool visit(MultiplicativeExpression0* n) { unimplementedVisitor("visit(MultiplicativeExpression0)"); return true; }
        virtual  void endVisit(MultiplicativeExpression0* n) { unimplementedVisitor("endVisit(MultiplicativeExpression0)"); }

        virtual   bool visit(MultiplicativeExpression1* n) { unimplementedVisitor("visit(MultiplicativeExpression1)"); return true; }
        virtual  void endVisit(MultiplicativeExpression1* n) { unimplementedVisitor("endVisit(MultiplicativeExpression1)"); }

        virtual   bool visit(MultiplicativeExpression2* n) { unimplementedVisitor("visit(MultiplicativeExpression2)"); return true; }
        virtual  void endVisit(MultiplicativeExpression2* n) { unimplementedVisitor("endVisit(MultiplicativeExpression2)"); }

        virtual   bool visit(AdditiveExpression0* n) { unimplementedVisitor("visit(AdditiveExpression0)"); return true; }
        virtual  void endVisit(AdditiveExpression0* n) { unimplementedVisitor("endVisit(AdditiveExpression0)"); }

        virtual   bool visit(AdditiveExpression1* n) { unimplementedVisitor("visit(AdditiveExpression1)"); return true; }
        virtual  void endVisit(AdditiveExpression1* n) { unimplementedVisitor("endVisit(AdditiveExpression1)"); }

        virtual   bool visit(ShiftExpression0* n) { unimplementedVisitor("visit(ShiftExpression0)"); return true; }
        virtual  void endVisit(ShiftExpression0* n) { unimplementedVisitor("endVisit(ShiftExpression0)"); }

        virtual   bool visit(ShiftExpression1* n) { unimplementedVisitor("visit(ShiftExpression1)"); return true; }
        virtual  void endVisit(ShiftExpression1* n) { unimplementedVisitor("endVisit(ShiftExpression1)"); }

        virtual   bool visit(ShiftExpression2* n) { unimplementedVisitor("visit(ShiftExpression2)"); return true; }
        virtual  void endVisit(ShiftExpression2* n) { unimplementedVisitor("endVisit(ShiftExpression2)"); }

        virtual   bool visit(RelationalExpression0* n) { unimplementedVisitor("visit(RelationalExpression0)"); return true; }
        virtual  void endVisit(RelationalExpression0* n) { unimplementedVisitor("endVisit(RelationalExpression0)"); }

        virtual   bool visit(RelationalExpression1* n) { unimplementedVisitor("visit(RelationalExpression1)"); return true; }
        virtual  void endVisit(RelationalExpression1* n) { unimplementedVisitor("endVisit(RelationalExpression1)"); }

        virtual   bool visit(RelationalExpression2* n) { unimplementedVisitor("visit(RelationalExpression2)"); return true; }
        virtual  void endVisit(RelationalExpression2* n) { unimplementedVisitor("endVisit(RelationalExpression2)"); }

        virtual   bool visit(RelationalExpression3* n) { unimplementedVisitor("visit(RelationalExpression3)"); return true; }
        virtual  void endVisit(RelationalExpression3* n) { unimplementedVisitor("endVisit(RelationalExpression3)"); }

        virtual   bool visit(RelationalExpression4* n) { unimplementedVisitor("visit(RelationalExpression4)"); return true; }
        virtual  void endVisit(RelationalExpression4* n) { unimplementedVisitor("endVisit(RelationalExpression4)"); }

        virtual   bool visit(EqualityExpression0* n) { unimplementedVisitor("visit(EqualityExpression0)"); return true; }
        virtual  void endVisit(EqualityExpression0* n) { unimplementedVisitor("endVisit(EqualityExpression0)"); }

        virtual   bool visit(EqualityExpression1* n) { unimplementedVisitor("visit(EqualityExpression1)"); return true; }
        virtual  void endVisit(EqualityExpression1* n) { unimplementedVisitor("endVisit(EqualityExpression1)"); }

        virtual   bool visit(AssignmentOperator0* n) { unimplementedVisitor("visit(AssignmentOperator0)"); return true; }
        virtual  void endVisit(AssignmentOperator0* n) { unimplementedVisitor("endVisit(AssignmentOperator0)"); }

        virtual   bool visit(AssignmentOperator1* n) { unimplementedVisitor("visit(AssignmentOperator1)"); return true; }
        virtual  void endVisit(AssignmentOperator1* n) { unimplementedVisitor("endVisit(AssignmentOperator1)"); }

        virtual   bool visit(AssignmentOperator2* n) { unimplementedVisitor("visit(AssignmentOperator2)"); return true; }
        virtual  void endVisit(AssignmentOperator2* n) { unimplementedVisitor("endVisit(AssignmentOperator2)"); }

        virtual   bool visit(AssignmentOperator3* n) { unimplementedVisitor("visit(AssignmentOperator3)"); return true; }
        virtual  void endVisit(AssignmentOperator3* n) { unimplementedVisitor("endVisit(AssignmentOperator3)"); }

        virtual   bool visit(AssignmentOperator4* n) { unimplementedVisitor("visit(AssignmentOperator4)"); return true; }
        virtual  void endVisit(AssignmentOperator4* n) { unimplementedVisitor("endVisit(AssignmentOperator4)"); }

        virtual   bool visit(AssignmentOperator5* n) { unimplementedVisitor("visit(AssignmentOperator5)"); return true; }
        virtual  void endVisit(AssignmentOperator5* n) { unimplementedVisitor("endVisit(AssignmentOperator5)"); }

        virtual   bool visit(AssignmentOperator6* n) { unimplementedVisitor("visit(AssignmentOperator6)"); return true; }
        virtual  void endVisit(AssignmentOperator6* n) { unimplementedVisitor("endVisit(AssignmentOperator6)"); }

        virtual   bool visit(AssignmentOperator7* n) { unimplementedVisitor("visit(AssignmentOperator7)"); return true; }
        virtual  void endVisit(AssignmentOperator7* n) { unimplementedVisitor("endVisit(AssignmentOperator7)"); }

        virtual   bool visit(AssignmentOperator8* n) { unimplementedVisitor("visit(AssignmentOperator8)"); return true; }
        virtual  void endVisit(AssignmentOperator8* n) { unimplementedVisitor("endVisit(AssignmentOperator8)"); }

        virtual   bool visit(AssignmentOperator9* n) { unimplementedVisitor("visit(AssignmentOperator9)"); return true; }
        virtual  void endVisit(AssignmentOperator9* n) { unimplementedVisitor("endVisit(AssignmentOperator9)"); }

        virtual   bool visit(AssignmentOperator10* n) { unimplementedVisitor("visit(AssignmentOperator10)"); return true; }
        virtual  void endVisit(AssignmentOperator10* n) { unimplementedVisitor("endVisit(AssignmentOperator10)"); }

        virtual   bool visit(AssignmentOperator11* n) { unimplementedVisitor("visit(AssignmentOperator11)"); return true; }
        virtual  void endVisit(AssignmentOperator11* n) { unimplementedVisitor("endVisit(AssignmentOperator11)"); }


        virtual   bool visit(Ast* n)
        {
            if (dynamic_cast<AstToken*>(n)) return visit((AstToken*)n);
            if (dynamic_cast<identifier*>(n)) return visit((identifier*)n);
            if (dynamic_cast<PrimitiveType*>(n)) return visit((PrimitiveType*)n);
            if (dynamic_cast<ClassType*>(n)) return visit((ClassType*)n);
            if (dynamic_cast<InterfaceType*>(n)) return visit((InterfaceType*)n);
            if (dynamic_cast<TypeName*>(n)) return visit((TypeName*)n);
            if (dynamic_cast<ArrayType*>(n)) return visit((ArrayType*)n);
            if (dynamic_cast<TypeParameter*>(n)) return visit((TypeParameter*)n);
            if (dynamic_cast<TypeBound*>(n)) return visit((TypeBound*)n);
            if (dynamic_cast<AdditionalBoundList*>(n)) return visit((AdditionalBoundList*)n);
            if (dynamic_cast<AdditionalBound*>(n)) return visit((AdditionalBound*)n);
            if (dynamic_cast<TypeArguments*>(n)) return visit((TypeArguments*)n);
            if (dynamic_cast<ActualTypeArgumentList*>(n)) return visit((ActualTypeArgumentList*)n);
            if (dynamic_cast<Wildcard*>(n)) return visit((Wildcard*)n);
            if (dynamic_cast<PackageName*>(n)) return visit((PackageName*)n);
            if (dynamic_cast<ExpressionName*>(n)) return visit((ExpressionName*)n);
            if (dynamic_cast<MethodName*>(n)) return visit((MethodName*)n);
            if (dynamic_cast<PackageOrTypeName*>(n)) return visit((PackageOrTypeName*)n);
            if (dynamic_cast<AmbiguousName*>(n)) return visit((AmbiguousName*)n);
            if (dynamic_cast<CompilationUnit*>(n)) return visit((CompilationUnit*)n);
            if (dynamic_cast<ImportDeclarations*>(n)) return visit((ImportDeclarations*)n);
            if (dynamic_cast<TypeDeclarations*>(n)) return visit((TypeDeclarations*)n);
            if (dynamic_cast<PackageDeclaration*>(n)) return visit((PackageDeclaration*)n);
            if (dynamic_cast<SingleTypeImportDeclaration*>(n)) return visit((SingleTypeImportDeclaration*)n);
            if (dynamic_cast<TypeImportOnDemandDeclaration*>(n)) return visit((TypeImportOnDemandDeclaration*)n);
            if (dynamic_cast<SingleStaticImportDeclaration*>(n)) return visit((SingleStaticImportDeclaration*)n);
            if (dynamic_cast<StaticImportOnDemandDeclaration*>(n)) return visit((StaticImportOnDemandDeclaration*)n);
            if (dynamic_cast<TypeDeclaration*>(n)) return visit((TypeDeclaration*)n);
            if (dynamic_cast<NormalClassDeclaration*>(n)) return visit((NormalClassDeclaration*)n);
            if (dynamic_cast<ClassModifiers*>(n)) return visit((ClassModifiers*)n);
            if (dynamic_cast<TypeParameters*>(n)) return visit((TypeParameters*)n);
            if (dynamic_cast<TypeParameterList*>(n)) return visit((TypeParameterList*)n);
            if (dynamic_cast<Super*>(n)) return visit((Super*)n);
            if (dynamic_cast<Interfaces*>(n)) return visit((Interfaces*)n);
            if (dynamic_cast<InterfaceTypeList*>(n)) return visit((InterfaceTypeList*)n);
            if (dynamic_cast<ClassBody*>(n)) return visit((ClassBody*)n);
            if (dynamic_cast<ClassBodyDeclarations*>(n)) return visit((ClassBodyDeclarations*)n);
            if (dynamic_cast<ClassMemberDeclaration*>(n)) return visit((ClassMemberDeclaration*)n);
            if (dynamic_cast<FieldDeclaration*>(n)) return visit((FieldDeclaration*)n);
            if (dynamic_cast<VariableDeclarators*>(n)) return visit((VariableDeclarators*)n);
            if (dynamic_cast<VariableDeclarator*>(n)) return visit((VariableDeclarator*)n);
            if (dynamic_cast<VariableDeclaratorId*>(n)) return visit((VariableDeclaratorId*)n);
            if (dynamic_cast<FieldModifiers*>(n)) return visit((FieldModifiers*)n);
            if (dynamic_cast<MethodDeclaration*>(n)) return visit((MethodDeclaration*)n);
            if (dynamic_cast<MethodHeader*>(n)) return visit((MethodHeader*)n);
            if (dynamic_cast<ResultType*>(n)) return visit((ResultType*)n);
            if (dynamic_cast<FormalParameterList*>(n)) return visit((FormalParameterList*)n);
            if (dynamic_cast<FormalParameters*>(n)) return visit((FormalParameters*)n);
            if (dynamic_cast<FormalParameter*>(n)) return visit((FormalParameter*)n);
            if (dynamic_cast<VariableModifiers*>(n)) return visit((VariableModifiers*)n);
            if (dynamic_cast<VariableModifier*>(n)) return visit((VariableModifier*)n);
            if (dynamic_cast<LastFormalParameter*>(n)) return visit((LastFormalParameter*)n);
            if (dynamic_cast<MethodModifiers*>(n)) return visit((MethodModifiers*)n);
            if (dynamic_cast<Throws*>(n)) return visit((Throws*)n);
            if (dynamic_cast<ExceptionTypeList*>(n)) return visit((ExceptionTypeList*)n);
            if (dynamic_cast<MethodBody*>(n)) return visit((MethodBody*)n);
            if (dynamic_cast<StaticInitializer*>(n)) return visit((StaticInitializer*)n);
            if (dynamic_cast<ConstructorDeclaration*>(n)) return visit((ConstructorDeclaration*)n);
            if (dynamic_cast<ConstructorDeclarator*>(n)) return visit((ConstructorDeclarator*)n);
            if (dynamic_cast<ConstructorModifiers*>(n)) return visit((ConstructorModifiers*)n);
            if (dynamic_cast<ConstructorBody*>(n)) return visit((ConstructorBody*)n);
            if (dynamic_cast<EnumDeclaration*>(n)) return visit((EnumDeclaration*)n);
            if (dynamic_cast<EnumBody*>(n)) return visit((EnumBody*)n);
            if (dynamic_cast<EnumConstants*>(n)) return visit((EnumConstants*)n);
            if (dynamic_cast<EnumConstant*>(n)) return visit((EnumConstant*)n);
            if (dynamic_cast<Arguments*>(n)) return visit((Arguments*)n);
            if (dynamic_cast<EnumBodyDeclarations*>(n)) return visit((EnumBodyDeclarations*)n);
            if (dynamic_cast<NormalInterfaceDeclaration*>(n)) return visit((NormalInterfaceDeclaration*)n);
            if (dynamic_cast<InterfaceModifiers*>(n)) return visit((InterfaceModifiers*)n);
            if (dynamic_cast<InterfaceBody*>(n)) return visit((InterfaceBody*)n);
            if (dynamic_cast<InterfaceMemberDeclarations*>(n)) return visit((InterfaceMemberDeclarations*)n);
            if (dynamic_cast<InterfaceMemberDeclaration*>(n)) return visit((InterfaceMemberDeclaration*)n);
            if (dynamic_cast<ConstantDeclaration*>(n)) return visit((ConstantDeclaration*)n);
            if (dynamic_cast<ConstantModifiers*>(n)) return visit((ConstantModifiers*)n);
            if (dynamic_cast<AbstractMethodDeclaration*>(n)) return visit((AbstractMethodDeclaration*)n);
            if (dynamic_cast<AbstractMethodModifiers*>(n)) return visit((AbstractMethodModifiers*)n);
            if (dynamic_cast<AnnotationTypeDeclaration*>(n)) return visit((AnnotationTypeDeclaration*)n);
            if (dynamic_cast<AnnotationTypeBody*>(n)) return visit((AnnotationTypeBody*)n);
            if (dynamic_cast<AnnotationTypeElementDeclarations*>(n)) return visit((AnnotationTypeElementDeclarations*)n);
            if (dynamic_cast<DefaultValue*>(n)) return visit((DefaultValue*)n);
            if (dynamic_cast<Annotations*>(n)) return visit((Annotations*)n);
            if (dynamic_cast<NormalAnnotation*>(n)) return visit((NormalAnnotation*)n);
            if (dynamic_cast<ElementValuePairs*>(n)) return visit((ElementValuePairs*)n);
            if (dynamic_cast<ElementValuePair*>(n)) return visit((ElementValuePair*)n);
            if (dynamic_cast<ElementValueArrayInitializer*>(n)) return visit((ElementValueArrayInitializer*)n);
            if (dynamic_cast<ElementValues*>(n)) return visit((ElementValues*)n);
            if (dynamic_cast<MarkerAnnotation*>(n)) return visit((MarkerAnnotation*)n);
            if (dynamic_cast<SingleElementAnnotation*>(n)) return visit((SingleElementAnnotation*)n);
            if (dynamic_cast<ArrayInitializer*>(n)) return visit((ArrayInitializer*)n);
            if (dynamic_cast<VariableInitializers*>(n)) return visit((VariableInitializers*)n);
            if (dynamic_cast<Block*>(n)) return visit((Block*)n);
            if (dynamic_cast<BlockStatements*>(n)) return visit((BlockStatements*)n);
            if (dynamic_cast<LocalVariableDeclarationStatement*>(n)) return visit((LocalVariableDeclarationStatement*)n);
            if (dynamic_cast<LocalVariableDeclaration*>(n)) return visit((LocalVariableDeclaration*)n);
            if (dynamic_cast<IfThenStatement*>(n)) return visit((IfThenStatement*)n);
            if (dynamic_cast<IfThenElseStatement*>(n)) return visit((IfThenElseStatement*)n);
            if (dynamic_cast<IfThenElseStatementNoShortIf*>(n)) return visit((IfThenElseStatementNoShortIf*)n);
            if (dynamic_cast<EmptyStatement*>(n)) return visit((EmptyStatement*)n);
            if (dynamic_cast<LabeledStatement*>(n)) return visit((LabeledStatement*)n);
            if (dynamic_cast<LabeledStatementNoShortIf*>(n)) return visit((LabeledStatementNoShortIf*)n);
            if (dynamic_cast<ExpressionStatement*>(n)) return visit((ExpressionStatement*)n);
            if (dynamic_cast<SwitchStatement*>(n)) return visit((SwitchStatement*)n);
            if (dynamic_cast<SwitchBlock*>(n)) return visit((SwitchBlock*)n);
            if (dynamic_cast<SwitchBlockStatementGroups*>(n)) return visit((SwitchBlockStatementGroups*)n);
            if (dynamic_cast<SwitchBlockStatementGroup*>(n)) return visit((SwitchBlockStatementGroup*)n);
            if (dynamic_cast<SwitchLabels*>(n)) return visit((SwitchLabels*)n);
            if (dynamic_cast<WhileStatement*>(n)) return visit((WhileStatement*)n);
            if (dynamic_cast<WhileStatementNoShortIf*>(n)) return visit((WhileStatementNoShortIf*)n);
            if (dynamic_cast<DoStatement*>(n)) return visit((DoStatement*)n);
            if (dynamic_cast<BasicForStatement*>(n)) return visit((BasicForStatement*)n);
            if (dynamic_cast<ForStatementNoShortIf*>(n)) return visit((ForStatementNoShortIf*)n);
            if (dynamic_cast<StatementExpressionList*>(n)) return visit((StatementExpressionList*)n);
            if (dynamic_cast<EnhancedForStatement*>(n)) return visit((EnhancedForStatement*)n);
            if (dynamic_cast<BreakStatement*>(n)) return visit((BreakStatement*)n);
            if (dynamic_cast<ContinueStatement*>(n)) return visit((ContinueStatement*)n);
            if (dynamic_cast<ReturnStatement*>(n)) return visit((ReturnStatement*)n);
            if (dynamic_cast<ThrowStatement*>(n)) return visit((ThrowStatement*)n);
            if (dynamic_cast<SynchronizedStatement*>(n)) return visit((SynchronizedStatement*)n);
            if (dynamic_cast<Catches*>(n)) return visit((Catches*)n);
            if (dynamic_cast<CatchClause*>(n)) return visit((CatchClause*)n);
            if (dynamic_cast<Finally*>(n)) return visit((Finally*)n);
            if (dynamic_cast<ArgumentList*>(n)) return visit((ArgumentList*)n);
            if (dynamic_cast<DimExprs*>(n)) return visit((DimExprs*)n);
            if (dynamic_cast<DimExpr*>(n)) return visit((DimExpr*)n);
            if (dynamic_cast<PostIncrementExpression*>(n)) return visit((PostIncrementExpression*)n);
            if (dynamic_cast<PostDecrementExpression*>(n)) return visit((PostDecrementExpression*)n);
            if (dynamic_cast<PreIncrementExpression*>(n)) return visit((PreIncrementExpression*)n);
            if (dynamic_cast<PreDecrementExpression*>(n)) return visit((PreDecrementExpression*)n);
            if (dynamic_cast<AndExpression*>(n)) return visit((AndExpression*)n);
            if (dynamic_cast<ExclusiveOrExpression*>(n)) return visit((ExclusiveOrExpression*)n);
            if (dynamic_cast<InclusiveOrExpression*>(n)) return visit((InclusiveOrExpression*)n);
            if (dynamic_cast<ConditionalAndExpression*>(n)) return visit((ConditionalAndExpression*)n);
            if (dynamic_cast<ConditionalOrExpression*>(n)) return visit((ConditionalOrExpression*)n);
            if (dynamic_cast<ConditionalExpression*>(n)) return visit((ConditionalExpression*)n);
            if (dynamic_cast<Assignment*>(n)) return visit((Assignment*)n);
            if (dynamic_cast<Commaopt*>(n)) return visit((Commaopt*)n);
            if (dynamic_cast<Ellipsisopt*>(n)) return visit((Ellipsisopt*)n);
            if (dynamic_cast<LPGUserAction0*>(n)) return visit((LPGUserAction0*)n);
            if (dynamic_cast<LPGUserAction1*>(n)) return visit((LPGUserAction1*)n);
            if (dynamic_cast<LPGUserAction2*>(n)) return visit((LPGUserAction2*)n);
            if (dynamic_cast<LPGUserAction3*>(n)) return visit((LPGUserAction3*)n);
            if (dynamic_cast<LPGUserAction4*>(n)) return visit((LPGUserAction4*)n);
            if (dynamic_cast<IntegralType0*>(n)) return visit((IntegralType0*)n);
            if (dynamic_cast<IntegralType1*>(n)) return visit((IntegralType1*)n);
            if (dynamic_cast<IntegralType2*>(n)) return visit((IntegralType2*)n);
            if (dynamic_cast<IntegralType3*>(n)) return visit((IntegralType3*)n);
            if (dynamic_cast<IntegralType4*>(n)) return visit((IntegralType4*)n);
            if (dynamic_cast<FloatingPointType0*>(n)) return visit((FloatingPointType0*)n);
            if (dynamic_cast<FloatingPointType1*>(n)) return visit((FloatingPointType1*)n);
            if (dynamic_cast<WildcardBounds0*>(n)) return visit((WildcardBounds0*)n);
            if (dynamic_cast<WildcardBounds1*>(n)) return visit((WildcardBounds1*)n);
            if (dynamic_cast<ClassModifier0*>(n)) return visit((ClassModifier0*)n);
            if (dynamic_cast<ClassModifier1*>(n)) return visit((ClassModifier1*)n);
            if (dynamic_cast<ClassModifier2*>(n)) return visit((ClassModifier2*)n);
            if (dynamic_cast<ClassModifier3*>(n)) return visit((ClassModifier3*)n);
            if (dynamic_cast<ClassModifier4*>(n)) return visit((ClassModifier4*)n);
            if (dynamic_cast<ClassModifier5*>(n)) return visit((ClassModifier5*)n);
            if (dynamic_cast<ClassModifier6*>(n)) return visit((ClassModifier6*)n);
            if (dynamic_cast<FieldModifier0*>(n)) return visit((FieldModifier0*)n);
            if (dynamic_cast<FieldModifier1*>(n)) return visit((FieldModifier1*)n);
            if (dynamic_cast<FieldModifier2*>(n)) return visit((FieldModifier2*)n);
            if (dynamic_cast<FieldModifier3*>(n)) return visit((FieldModifier3*)n);
            if (dynamic_cast<FieldModifier4*>(n)) return visit((FieldModifier4*)n);
            if (dynamic_cast<FieldModifier5*>(n)) return visit((FieldModifier5*)n);
            if (dynamic_cast<FieldModifier6*>(n)) return visit((FieldModifier6*)n);
            if (dynamic_cast<MethodDeclarator0*>(n)) return visit((MethodDeclarator0*)n);
            if (dynamic_cast<MethodDeclarator1*>(n)) return visit((MethodDeclarator1*)n);
            if (dynamic_cast<MethodModifier0*>(n)) return visit((MethodModifier0*)n);
            if (dynamic_cast<MethodModifier1*>(n)) return visit((MethodModifier1*)n);
            if (dynamic_cast<MethodModifier2*>(n)) return visit((MethodModifier2*)n);
            if (dynamic_cast<MethodModifier3*>(n)) return visit((MethodModifier3*)n);
            if (dynamic_cast<MethodModifier4*>(n)) return visit((MethodModifier4*)n);
            if (dynamic_cast<MethodModifier5*>(n)) return visit((MethodModifier5*)n);
            if (dynamic_cast<MethodModifier6*>(n)) return visit((MethodModifier6*)n);
            if (dynamic_cast<MethodModifier7*>(n)) return visit((MethodModifier7*)n);
            if (dynamic_cast<MethodModifier8*>(n)) return visit((MethodModifier8*)n);
            if (dynamic_cast<ConstructorModifier0*>(n)) return visit((ConstructorModifier0*)n);
            if (dynamic_cast<ConstructorModifier1*>(n)) return visit((ConstructorModifier1*)n);
            if (dynamic_cast<ConstructorModifier2*>(n)) return visit((ConstructorModifier2*)n);
            if (dynamic_cast<ExplicitConstructorInvocation0*>(n)) return visit((ExplicitConstructorInvocation0*)n);
            if (dynamic_cast<ExplicitConstructorInvocation1*>(n)) return visit((ExplicitConstructorInvocation1*)n);
            if (dynamic_cast<ExplicitConstructorInvocation2*>(n)) return visit((ExplicitConstructorInvocation2*)n);
            if (dynamic_cast<InterfaceModifier0*>(n)) return visit((InterfaceModifier0*)n);
            if (dynamic_cast<InterfaceModifier1*>(n)) return visit((InterfaceModifier1*)n);
            if (dynamic_cast<InterfaceModifier2*>(n)) return visit((InterfaceModifier2*)n);
            if (dynamic_cast<InterfaceModifier3*>(n)) return visit((InterfaceModifier3*)n);
            if (dynamic_cast<InterfaceModifier4*>(n)) return visit((InterfaceModifier4*)n);
            if (dynamic_cast<InterfaceModifier5*>(n)) return visit((InterfaceModifier5*)n);
            if (dynamic_cast<ExtendsInterfaces0*>(n)) return visit((ExtendsInterfaces0*)n);
            if (dynamic_cast<ExtendsInterfaces1*>(n)) return visit((ExtendsInterfaces1*)n);
            if (dynamic_cast<ConstantModifier0*>(n)) return visit((ConstantModifier0*)n);
            if (dynamic_cast<ConstantModifier1*>(n)) return visit((ConstantModifier1*)n);
            if (dynamic_cast<ConstantModifier2*>(n)) return visit((ConstantModifier2*)n);
            if (dynamic_cast<AbstractMethodModifier0*>(n)) return visit((AbstractMethodModifier0*)n);
            if (dynamic_cast<AbstractMethodModifier1*>(n)) return visit((AbstractMethodModifier1*)n);
            if (dynamic_cast<AnnotationTypeElementDeclaration0*>(n)) return visit((AnnotationTypeElementDeclaration0*)n);
            if (dynamic_cast<AnnotationTypeElementDeclaration1*>(n)) return visit((AnnotationTypeElementDeclaration1*)n);
            if (dynamic_cast<AssertStatement0*>(n)) return visit((AssertStatement0*)n);
            if (dynamic_cast<AssertStatement1*>(n)) return visit((AssertStatement1*)n);
            if (dynamic_cast<SwitchLabel0*>(n)) return visit((SwitchLabel0*)n);
            if (dynamic_cast<SwitchLabel1*>(n)) return visit((SwitchLabel1*)n);
            if (dynamic_cast<SwitchLabel2*>(n)) return visit((SwitchLabel2*)n);
            if (dynamic_cast<TryStatement0*>(n)) return visit((TryStatement0*)n);
            if (dynamic_cast<TryStatement1*>(n)) return visit((TryStatement1*)n);
            if (dynamic_cast<PrimaryNoNewArray0*>(n)) return visit((PrimaryNoNewArray0*)n);
            if (dynamic_cast<PrimaryNoNewArray1*>(n)) return visit((PrimaryNoNewArray1*)n);
            if (dynamic_cast<PrimaryNoNewArray2*>(n)) return visit((PrimaryNoNewArray2*)n);
            if (dynamic_cast<PrimaryNoNewArray3*>(n)) return visit((PrimaryNoNewArray3*)n);
            if (dynamic_cast<PrimaryNoNewArray4*>(n)) return visit((PrimaryNoNewArray4*)n);
            if (dynamic_cast<Literal0*>(n)) return visit((Literal0*)n);
            if (dynamic_cast<Literal1*>(n)) return visit((Literal1*)n);
            if (dynamic_cast<Literal2*>(n)) return visit((Literal2*)n);
            if (dynamic_cast<Literal3*>(n)) return visit((Literal3*)n);
            if (dynamic_cast<Literal4*>(n)) return visit((Literal4*)n);
            if (dynamic_cast<Literal5*>(n)) return visit((Literal5*)n);
            if (dynamic_cast<Literal6*>(n)) return visit((Literal6*)n);
            if (dynamic_cast<BooleanLiteral0*>(n)) return visit((BooleanLiteral0*)n);
            if (dynamic_cast<BooleanLiteral1*>(n)) return visit((BooleanLiteral1*)n);
            if (dynamic_cast<ClassInstanceCreationExpression0*>(n)) return visit((ClassInstanceCreationExpression0*)n);
            if (dynamic_cast<ClassInstanceCreationExpression1*>(n)) return visit((ClassInstanceCreationExpression1*)n);
            if (dynamic_cast<ArrayCreationExpression0*>(n)) return visit((ArrayCreationExpression0*)n);
            if (dynamic_cast<ArrayCreationExpression1*>(n)) return visit((ArrayCreationExpression1*)n);
            if (dynamic_cast<ArrayCreationExpression2*>(n)) return visit((ArrayCreationExpression2*)n);
            if (dynamic_cast<ArrayCreationExpression3*>(n)) return visit((ArrayCreationExpression3*)n);
            if (dynamic_cast<Dims0*>(n)) return visit((Dims0*)n);
            if (dynamic_cast<Dims1*>(n)) return visit((Dims1*)n);
            if (dynamic_cast<FieldAccess0*>(n)) return visit((FieldAccess0*)n);
            if (dynamic_cast<FieldAccess1*>(n)) return visit((FieldAccess1*)n);
            if (dynamic_cast<FieldAccess2*>(n)) return visit((FieldAccess2*)n);
            if (dynamic_cast<MethodInvocation0*>(n)) return visit((MethodInvocation0*)n);
            if (dynamic_cast<MethodInvocation1*>(n)) return visit((MethodInvocation1*)n);
            if (dynamic_cast<MethodInvocation2*>(n)) return visit((MethodInvocation2*)n);
            if (dynamic_cast<MethodInvocation3*>(n)) return visit((MethodInvocation3*)n);
            if (dynamic_cast<MethodInvocation4*>(n)) return visit((MethodInvocation4*)n);
            if (dynamic_cast<ArrayAccess0*>(n)) return visit((ArrayAccess0*)n);
            if (dynamic_cast<ArrayAccess1*>(n)) return visit((ArrayAccess1*)n);
            if (dynamic_cast<UnaryExpression0*>(n)) return visit((UnaryExpression0*)n);
            if (dynamic_cast<UnaryExpression1*>(n)) return visit((UnaryExpression1*)n);
            if (dynamic_cast<UnaryExpressionNotPlusMinus0*>(n)) return visit((UnaryExpressionNotPlusMinus0*)n);
            if (dynamic_cast<UnaryExpressionNotPlusMinus1*>(n)) return visit((UnaryExpressionNotPlusMinus1*)n);
            if (dynamic_cast<CastExpression0*>(n)) return visit((CastExpression0*)n);
            if (dynamic_cast<CastExpression1*>(n)) return visit((CastExpression1*)n);
            if (dynamic_cast<MultiplicativeExpression0*>(n)) return visit((MultiplicativeExpression0*)n);
            if (dynamic_cast<MultiplicativeExpression1*>(n)) return visit((MultiplicativeExpression1*)n);
            if (dynamic_cast<MultiplicativeExpression2*>(n)) return visit((MultiplicativeExpression2*)n);
            if (dynamic_cast<AdditiveExpression0*>(n)) return visit((AdditiveExpression0*)n);
            if (dynamic_cast<AdditiveExpression1*>(n)) return visit((AdditiveExpression1*)n);
            if (dynamic_cast<ShiftExpression0*>(n)) return visit((ShiftExpression0*)n);
            if (dynamic_cast<ShiftExpression1*>(n)) return visit((ShiftExpression1*)n);
            if (dynamic_cast<ShiftExpression2*>(n)) return visit((ShiftExpression2*)n);
            if (dynamic_cast<RelationalExpression0*>(n)) return visit((RelationalExpression0*)n);
            if (dynamic_cast<RelationalExpression1*>(n)) return visit((RelationalExpression1*)n);
            if (dynamic_cast<RelationalExpression2*>(n)) return visit((RelationalExpression2*)n);
            if (dynamic_cast<RelationalExpression3*>(n)) return visit((RelationalExpression3*)n);
            if (dynamic_cast<RelationalExpression4*>(n)) return visit((RelationalExpression4*)n);
            if (dynamic_cast<EqualityExpression0*>(n)) return visit((EqualityExpression0*)n);
            if (dynamic_cast<EqualityExpression1*>(n)) return visit((EqualityExpression1*)n);
            if (dynamic_cast<AssignmentOperator0*>(n)) return visit((AssignmentOperator0*)n);
            if (dynamic_cast<AssignmentOperator1*>(n)) return visit((AssignmentOperator1*)n);
            if (dynamic_cast<AssignmentOperator2*>(n)) return visit((AssignmentOperator2*)n);
            if (dynamic_cast<AssignmentOperator3*>(n)) return visit((AssignmentOperator3*)n);
            if (dynamic_cast<AssignmentOperator4*>(n)) return visit((AssignmentOperator4*)n);
            if (dynamic_cast<AssignmentOperator5*>(n)) return visit((AssignmentOperator5*)n);
            if (dynamic_cast<AssignmentOperator6*>(n)) return visit((AssignmentOperator6*)n);
            if (dynamic_cast<AssignmentOperator7*>(n)) return visit((AssignmentOperator7*)n);
            if (dynamic_cast<AssignmentOperator8*>(n)) return visit((AssignmentOperator8*)n);
            if (dynamic_cast<AssignmentOperator9*>(n)) return visit((AssignmentOperator9*)n);
            if (dynamic_cast<AssignmentOperator10*>(n)) return visit((AssignmentOperator10*)n);
            if (dynamic_cast<AssignmentOperator11*>(n)) return visit((AssignmentOperator11*)n);
            throw UnsupportedOperationException("visit(" + n->to_utf8_string() + ")");
        }
        virtual  void endVisit(Ast* n)
        {
            if (dynamic_cast<AstToken*>(n)) endVisit((AstToken*)n); return;
            if (dynamic_cast<identifier*>(n)) endVisit((identifier*)n); return;
            if (dynamic_cast<PrimitiveType*>(n)) endVisit((PrimitiveType*)n); return;
            if (dynamic_cast<ClassType*>(n)) endVisit((ClassType*)n); return;
            if (dynamic_cast<InterfaceType*>(n)) endVisit((InterfaceType*)n); return;
            if (dynamic_cast<TypeName*>(n)) endVisit((TypeName*)n); return;
            if (dynamic_cast<ArrayType*>(n)) endVisit((ArrayType*)n); return;
            if (dynamic_cast<TypeParameter*>(n)) endVisit((TypeParameter*)n); return;
            if (dynamic_cast<TypeBound*>(n)) endVisit((TypeBound*)n); return;
            if (dynamic_cast<AdditionalBoundList*>(n)) endVisit((AdditionalBoundList*)n); return;
            if (dynamic_cast<AdditionalBound*>(n)) endVisit((AdditionalBound*)n); return;
            if (dynamic_cast<TypeArguments*>(n)) endVisit((TypeArguments*)n); return;
            if (dynamic_cast<ActualTypeArgumentList*>(n)) endVisit((ActualTypeArgumentList*)n); return;
            if (dynamic_cast<Wildcard*>(n)) endVisit((Wildcard*)n); return;
            if (dynamic_cast<PackageName*>(n)) endVisit((PackageName*)n); return;
            if (dynamic_cast<ExpressionName*>(n)) endVisit((ExpressionName*)n); return;
            if (dynamic_cast<MethodName*>(n)) endVisit((MethodName*)n); return;
            if (dynamic_cast<PackageOrTypeName*>(n)) endVisit((PackageOrTypeName*)n); return;
            if (dynamic_cast<AmbiguousName*>(n)) endVisit((AmbiguousName*)n); return;
            if (dynamic_cast<CompilationUnit*>(n)) endVisit((CompilationUnit*)n); return;
            if (dynamic_cast<ImportDeclarations*>(n)) endVisit((ImportDeclarations*)n); return;
            if (dynamic_cast<TypeDeclarations*>(n)) endVisit((TypeDeclarations*)n); return;
            if (dynamic_cast<PackageDeclaration*>(n)) endVisit((PackageDeclaration*)n); return;
            if (dynamic_cast<SingleTypeImportDeclaration*>(n)) endVisit((SingleTypeImportDeclaration*)n); return;
            if (dynamic_cast<TypeImportOnDemandDeclaration*>(n)) endVisit((TypeImportOnDemandDeclaration*)n); return;
            if (dynamic_cast<SingleStaticImportDeclaration*>(n)) endVisit((SingleStaticImportDeclaration*)n); return;
            if (dynamic_cast<StaticImportOnDemandDeclaration*>(n)) endVisit((StaticImportOnDemandDeclaration*)n); return;
            if (dynamic_cast<TypeDeclaration*>(n)) endVisit((TypeDeclaration*)n); return;
            if (dynamic_cast<NormalClassDeclaration*>(n)) endVisit((NormalClassDeclaration*)n); return;
            if (dynamic_cast<ClassModifiers*>(n)) endVisit((ClassModifiers*)n); return;
            if (dynamic_cast<TypeParameters*>(n)) endVisit((TypeParameters*)n); return;
            if (dynamic_cast<TypeParameterList*>(n)) endVisit((TypeParameterList*)n); return;
            if (dynamic_cast<Super*>(n)) endVisit((Super*)n); return;
            if (dynamic_cast<Interfaces*>(n)) endVisit((Interfaces*)n); return;
            if (dynamic_cast<InterfaceTypeList*>(n)) endVisit((InterfaceTypeList*)n); return;
            if (dynamic_cast<ClassBody*>(n)) endVisit((ClassBody*)n); return;
            if (dynamic_cast<ClassBodyDeclarations*>(n)) endVisit((ClassBodyDeclarations*)n); return;
            if (dynamic_cast<ClassMemberDeclaration*>(n)) endVisit((ClassMemberDeclaration*)n); return;
            if (dynamic_cast<FieldDeclaration*>(n)) endVisit((FieldDeclaration*)n); return;
            if (dynamic_cast<VariableDeclarators*>(n)) endVisit((VariableDeclarators*)n); return;
            if (dynamic_cast<VariableDeclarator*>(n)) endVisit((VariableDeclarator*)n); return;
            if (dynamic_cast<VariableDeclaratorId*>(n)) endVisit((VariableDeclaratorId*)n); return;
            if (dynamic_cast<FieldModifiers*>(n)) endVisit((FieldModifiers*)n); return;
            if (dynamic_cast<MethodDeclaration*>(n)) endVisit((MethodDeclaration*)n); return;
            if (dynamic_cast<MethodHeader*>(n)) endVisit((MethodHeader*)n); return;
            if (dynamic_cast<ResultType*>(n)) endVisit((ResultType*)n); return;
            if (dynamic_cast<FormalParameterList*>(n)) endVisit((FormalParameterList*)n); return;
            if (dynamic_cast<FormalParameters*>(n)) endVisit((FormalParameters*)n); return;
            if (dynamic_cast<FormalParameter*>(n)) endVisit((FormalParameter*)n); return;
            if (dynamic_cast<VariableModifiers*>(n)) endVisit((VariableModifiers*)n); return;
            if (dynamic_cast<VariableModifier*>(n)) endVisit((VariableModifier*)n); return;
            if (dynamic_cast<LastFormalParameter*>(n)) endVisit((LastFormalParameter*)n); return;
            if (dynamic_cast<MethodModifiers*>(n)) endVisit((MethodModifiers*)n); return;
            if (dynamic_cast<Throws*>(n)) endVisit((Throws*)n); return;
            if (dynamic_cast<ExceptionTypeList*>(n)) endVisit((ExceptionTypeList*)n); return;
            if (dynamic_cast<MethodBody*>(n)) endVisit((MethodBody*)n); return;
            if (dynamic_cast<StaticInitializer*>(n)) endVisit((StaticInitializer*)n); return;
            if (dynamic_cast<ConstructorDeclaration*>(n)) endVisit((ConstructorDeclaration*)n); return;
            if (dynamic_cast<ConstructorDeclarator*>(n)) endVisit((ConstructorDeclarator*)n); return;
            if (dynamic_cast<ConstructorModifiers*>(n)) endVisit((ConstructorModifiers*)n); return;
            if (dynamic_cast<ConstructorBody*>(n)) endVisit((ConstructorBody*)n); return;
            if (dynamic_cast<EnumDeclaration*>(n)) endVisit((EnumDeclaration*)n); return;
            if (dynamic_cast<EnumBody*>(n)) endVisit((EnumBody*)n); return;
            if (dynamic_cast<EnumConstants*>(n)) endVisit((EnumConstants*)n); return;
            if (dynamic_cast<EnumConstant*>(n)) endVisit((EnumConstant*)n); return;
            if (dynamic_cast<Arguments*>(n)) endVisit((Arguments*)n); return;
            if (dynamic_cast<EnumBodyDeclarations*>(n)) endVisit((EnumBodyDeclarations*)n); return;
            if (dynamic_cast<NormalInterfaceDeclaration*>(n)) endVisit((NormalInterfaceDeclaration*)n); return;
            if (dynamic_cast<InterfaceModifiers*>(n)) endVisit((InterfaceModifiers*)n); return;
            if (dynamic_cast<InterfaceBody*>(n)) endVisit((InterfaceBody*)n); return;
            if (dynamic_cast<InterfaceMemberDeclarations*>(n)) endVisit((InterfaceMemberDeclarations*)n); return;
            if (dynamic_cast<InterfaceMemberDeclaration*>(n)) endVisit((InterfaceMemberDeclaration*)n); return;
            if (dynamic_cast<ConstantDeclaration*>(n)) endVisit((ConstantDeclaration*)n); return;
            if (dynamic_cast<ConstantModifiers*>(n)) endVisit((ConstantModifiers*)n); return;
            if (dynamic_cast<AbstractMethodDeclaration*>(n)) endVisit((AbstractMethodDeclaration*)n); return;
            if (dynamic_cast<AbstractMethodModifiers*>(n)) endVisit((AbstractMethodModifiers*)n); return;
            if (dynamic_cast<AnnotationTypeDeclaration*>(n)) endVisit((AnnotationTypeDeclaration*)n); return;
            if (dynamic_cast<AnnotationTypeBody*>(n)) endVisit((AnnotationTypeBody*)n); return;
            if (dynamic_cast<AnnotationTypeElementDeclarations*>(n)) endVisit((AnnotationTypeElementDeclarations*)n); return;
            if (dynamic_cast<DefaultValue*>(n)) endVisit((DefaultValue*)n); return;
            if (dynamic_cast<Annotations*>(n)) endVisit((Annotations*)n); return;
            if (dynamic_cast<NormalAnnotation*>(n)) endVisit((NormalAnnotation*)n); return;
            if (dynamic_cast<ElementValuePairs*>(n)) endVisit((ElementValuePairs*)n); return;
            if (dynamic_cast<ElementValuePair*>(n)) endVisit((ElementValuePair*)n); return;
            if (dynamic_cast<ElementValueArrayInitializer*>(n)) endVisit((ElementValueArrayInitializer*)n); return;
            if (dynamic_cast<ElementValues*>(n)) endVisit((ElementValues*)n); return;
            if (dynamic_cast<MarkerAnnotation*>(n)) endVisit((MarkerAnnotation*)n); return;
            if (dynamic_cast<SingleElementAnnotation*>(n)) endVisit((SingleElementAnnotation*)n); return;
            if (dynamic_cast<ArrayInitializer*>(n)) endVisit((ArrayInitializer*)n); return;
            if (dynamic_cast<VariableInitializers*>(n)) endVisit((VariableInitializers*)n); return;
            if (dynamic_cast<Block*>(n)) endVisit((Block*)n); return;
            if (dynamic_cast<BlockStatements*>(n)) endVisit((BlockStatements*)n); return;
            if (dynamic_cast<LocalVariableDeclarationStatement*>(n)) endVisit((LocalVariableDeclarationStatement*)n); return;
            if (dynamic_cast<LocalVariableDeclaration*>(n)) endVisit((LocalVariableDeclaration*)n); return;
            if (dynamic_cast<IfThenStatement*>(n)) endVisit((IfThenStatement*)n); return;
            if (dynamic_cast<IfThenElseStatement*>(n)) endVisit((IfThenElseStatement*)n); return;
            if (dynamic_cast<IfThenElseStatementNoShortIf*>(n)) endVisit((IfThenElseStatementNoShortIf*)n); return;
            if (dynamic_cast<EmptyStatement*>(n)) endVisit((EmptyStatement*)n); return;
            if (dynamic_cast<LabeledStatement*>(n)) endVisit((LabeledStatement*)n); return;
            if (dynamic_cast<LabeledStatementNoShortIf*>(n)) endVisit((LabeledStatementNoShortIf*)n); return;
            if (dynamic_cast<ExpressionStatement*>(n)) endVisit((ExpressionStatement*)n); return;
            if (dynamic_cast<SwitchStatement*>(n)) endVisit((SwitchStatement*)n); return;
            if (dynamic_cast<SwitchBlock*>(n)) endVisit((SwitchBlock*)n); return;
            if (dynamic_cast<SwitchBlockStatementGroups*>(n)) endVisit((SwitchBlockStatementGroups*)n); return;
            if (dynamic_cast<SwitchBlockStatementGroup*>(n)) endVisit((SwitchBlockStatementGroup*)n); return;
            if (dynamic_cast<SwitchLabels*>(n)) endVisit((SwitchLabels*)n); return;
            if (dynamic_cast<WhileStatement*>(n)) endVisit((WhileStatement*)n); return;
            if (dynamic_cast<WhileStatementNoShortIf*>(n)) endVisit((WhileStatementNoShortIf*)n); return;
            if (dynamic_cast<DoStatement*>(n)) endVisit((DoStatement*)n); return;
            if (dynamic_cast<BasicForStatement*>(n)) endVisit((BasicForStatement*)n); return;
            if (dynamic_cast<ForStatementNoShortIf*>(n)) endVisit((ForStatementNoShortIf*)n); return;
            if (dynamic_cast<StatementExpressionList*>(n)) endVisit((StatementExpressionList*)n); return;
            if (dynamic_cast<EnhancedForStatement*>(n)) endVisit((EnhancedForStatement*)n); return;
            if (dynamic_cast<BreakStatement*>(n)) endVisit((BreakStatement*)n); return;
            if (dynamic_cast<ContinueStatement*>(n)) endVisit((ContinueStatement*)n); return;
            if (dynamic_cast<ReturnStatement*>(n)) endVisit((ReturnStatement*)n); return;
            if (dynamic_cast<ThrowStatement*>(n)) endVisit((ThrowStatement*)n); return;
            if (dynamic_cast<SynchronizedStatement*>(n)) endVisit((SynchronizedStatement*)n); return;
            if (dynamic_cast<Catches*>(n)) endVisit((Catches*)n); return;
            if (dynamic_cast<CatchClause*>(n)) endVisit((CatchClause*)n); return;
            if (dynamic_cast<Finally*>(n)) endVisit((Finally*)n); return;
            if (dynamic_cast<ArgumentList*>(n)) endVisit((ArgumentList*)n); return;
            if (dynamic_cast<DimExprs*>(n)) endVisit((DimExprs*)n); return;
            if (dynamic_cast<DimExpr*>(n)) endVisit((DimExpr*)n); return;
            if (dynamic_cast<PostIncrementExpression*>(n)) endVisit((PostIncrementExpression*)n); return;
            if (dynamic_cast<PostDecrementExpression*>(n)) endVisit((PostDecrementExpression*)n); return;
            if (dynamic_cast<PreIncrementExpression*>(n)) endVisit((PreIncrementExpression*)n); return;
            if (dynamic_cast<PreDecrementExpression*>(n)) endVisit((PreDecrementExpression*)n); return;
            if (dynamic_cast<AndExpression*>(n)) endVisit((AndExpression*)n); return;
            if (dynamic_cast<ExclusiveOrExpression*>(n)) endVisit((ExclusiveOrExpression*)n); return;
            if (dynamic_cast<InclusiveOrExpression*>(n)) endVisit((InclusiveOrExpression*)n); return;
            if (dynamic_cast<ConditionalAndExpression*>(n)) endVisit((ConditionalAndExpression*)n); return;
            if (dynamic_cast<ConditionalOrExpression*>(n)) endVisit((ConditionalOrExpression*)n); return;
            if (dynamic_cast<ConditionalExpression*>(n)) endVisit((ConditionalExpression*)n); return;
            if (dynamic_cast<Assignment*>(n)) endVisit((Assignment*)n); return;
            if (dynamic_cast<Commaopt*>(n)) endVisit((Commaopt*)n); return;
            if (dynamic_cast<Ellipsisopt*>(n)) endVisit((Ellipsisopt*)n); return;
            if (dynamic_cast<LPGUserAction0*>(n)) endVisit((LPGUserAction0*)n); return;
            if (dynamic_cast<LPGUserAction1*>(n)) endVisit((LPGUserAction1*)n); return;
            if (dynamic_cast<LPGUserAction2*>(n)) endVisit((LPGUserAction2*)n); return;
            if (dynamic_cast<LPGUserAction3*>(n)) endVisit((LPGUserAction3*)n); return;
            if (dynamic_cast<LPGUserAction4*>(n)) endVisit((LPGUserAction4*)n); return;
            if (dynamic_cast<IntegralType0*>(n)) endVisit((IntegralType0*)n); return;
            if (dynamic_cast<IntegralType1*>(n)) endVisit((IntegralType1*)n); return;
            if (dynamic_cast<IntegralType2*>(n)) endVisit((IntegralType2*)n); return;
            if (dynamic_cast<IntegralType3*>(n)) endVisit((IntegralType3*)n); return;
            if (dynamic_cast<IntegralType4*>(n)) endVisit((IntegralType4*)n); return;
            if (dynamic_cast<FloatingPointType0*>(n)) endVisit((FloatingPointType0*)n); return;
            if (dynamic_cast<FloatingPointType1*>(n)) endVisit((FloatingPointType1*)n); return;
            if (dynamic_cast<WildcardBounds0*>(n)) endVisit((WildcardBounds0*)n); return;
            if (dynamic_cast<WildcardBounds1*>(n)) endVisit((WildcardBounds1*)n); return;
            if (dynamic_cast<ClassModifier0*>(n)) endVisit((ClassModifier0*)n); return;
            if (dynamic_cast<ClassModifier1*>(n)) endVisit((ClassModifier1*)n); return;
            if (dynamic_cast<ClassModifier2*>(n)) endVisit((ClassModifier2*)n); return;
            if (dynamic_cast<ClassModifier3*>(n)) endVisit((ClassModifier3*)n); return;
            if (dynamic_cast<ClassModifier4*>(n)) endVisit((ClassModifier4*)n); return;
            if (dynamic_cast<ClassModifier5*>(n)) endVisit((ClassModifier5*)n); return;
            if (dynamic_cast<ClassModifier6*>(n)) endVisit((ClassModifier6*)n); return;
            if (dynamic_cast<FieldModifier0*>(n)) endVisit((FieldModifier0*)n); return;
            if (dynamic_cast<FieldModifier1*>(n)) endVisit((FieldModifier1*)n); return;
            if (dynamic_cast<FieldModifier2*>(n)) endVisit((FieldModifier2*)n); return;
            if (dynamic_cast<FieldModifier3*>(n)) endVisit((FieldModifier3*)n); return;
            if (dynamic_cast<FieldModifier4*>(n)) endVisit((FieldModifier4*)n); return;
            if (dynamic_cast<FieldModifier5*>(n)) endVisit((FieldModifier5*)n); return;
            if (dynamic_cast<FieldModifier6*>(n)) endVisit((FieldModifier6*)n); return;
            if (dynamic_cast<MethodDeclarator0*>(n)) endVisit((MethodDeclarator0*)n); return;
            if (dynamic_cast<MethodDeclarator1*>(n)) endVisit((MethodDeclarator1*)n); return;
            if (dynamic_cast<MethodModifier0*>(n)) endVisit((MethodModifier0*)n); return;
            if (dynamic_cast<MethodModifier1*>(n)) endVisit((MethodModifier1*)n); return;
            if (dynamic_cast<MethodModifier2*>(n)) endVisit((MethodModifier2*)n); return;
            if (dynamic_cast<MethodModifier3*>(n)) endVisit((MethodModifier3*)n); return;
            if (dynamic_cast<MethodModifier4*>(n)) endVisit((MethodModifier4*)n); return;
            if (dynamic_cast<MethodModifier5*>(n)) endVisit((MethodModifier5*)n); return;
            if (dynamic_cast<MethodModifier6*>(n)) endVisit((MethodModifier6*)n); return;
            if (dynamic_cast<MethodModifier7*>(n)) endVisit((MethodModifier7*)n); return;
            if (dynamic_cast<MethodModifier8*>(n)) endVisit((MethodModifier8*)n); return;
            if (dynamic_cast<ConstructorModifier0*>(n)) endVisit((ConstructorModifier0*)n); return;
            if (dynamic_cast<ConstructorModifier1*>(n)) endVisit((ConstructorModifier1*)n); return;
            if (dynamic_cast<ConstructorModifier2*>(n)) endVisit((ConstructorModifier2*)n); return;
            if (dynamic_cast<ExplicitConstructorInvocation0*>(n)) endVisit((ExplicitConstructorInvocation0*)n); return;
            if (dynamic_cast<ExplicitConstructorInvocation1*>(n)) endVisit((ExplicitConstructorInvocation1*)n); return;
            if (dynamic_cast<ExplicitConstructorInvocation2*>(n)) endVisit((ExplicitConstructorInvocation2*)n); return;
            if (dynamic_cast<InterfaceModifier0*>(n)) endVisit((InterfaceModifier0*)n); return;
            if (dynamic_cast<InterfaceModifier1*>(n)) endVisit((InterfaceModifier1*)n); return;
            if (dynamic_cast<InterfaceModifier2*>(n)) endVisit((InterfaceModifier2*)n); return;
            if (dynamic_cast<InterfaceModifier3*>(n)) endVisit((InterfaceModifier3*)n); return;
            if (dynamic_cast<InterfaceModifier4*>(n)) endVisit((InterfaceModifier4*)n); return;
            if (dynamic_cast<InterfaceModifier5*>(n)) endVisit((InterfaceModifier5*)n); return;
            if (dynamic_cast<ExtendsInterfaces0*>(n)) endVisit((ExtendsInterfaces0*)n); return;
            if (dynamic_cast<ExtendsInterfaces1*>(n)) endVisit((ExtendsInterfaces1*)n); return;
            if (dynamic_cast<ConstantModifier0*>(n)) endVisit((ConstantModifier0*)n); return;
            if (dynamic_cast<ConstantModifier1*>(n)) endVisit((ConstantModifier1*)n); return;
            if (dynamic_cast<ConstantModifier2*>(n)) endVisit((ConstantModifier2*)n); return;
            if (dynamic_cast<AbstractMethodModifier0*>(n)) endVisit((AbstractMethodModifier0*)n); return;
            if (dynamic_cast<AbstractMethodModifier1*>(n)) endVisit((AbstractMethodModifier1*)n); return;
            if (dynamic_cast<AnnotationTypeElementDeclaration0*>(n)) endVisit((AnnotationTypeElementDeclaration0*)n); return;
            if (dynamic_cast<AnnotationTypeElementDeclaration1*>(n)) endVisit((AnnotationTypeElementDeclaration1*)n); return;
            if (dynamic_cast<AssertStatement0*>(n)) endVisit((AssertStatement0*)n); return;
            if (dynamic_cast<AssertStatement1*>(n)) endVisit((AssertStatement1*)n); return;
            if (dynamic_cast<SwitchLabel0*>(n)) endVisit((SwitchLabel0*)n); return;
            if (dynamic_cast<SwitchLabel1*>(n)) endVisit((SwitchLabel1*)n); return;
            if (dynamic_cast<SwitchLabel2*>(n)) endVisit((SwitchLabel2*)n); return;
            if (dynamic_cast<TryStatement0*>(n)) endVisit((TryStatement0*)n); return;
            if (dynamic_cast<TryStatement1*>(n)) endVisit((TryStatement1*)n); return;
            if (dynamic_cast<PrimaryNoNewArray0*>(n)) endVisit((PrimaryNoNewArray0*)n); return;
            if (dynamic_cast<PrimaryNoNewArray1*>(n)) endVisit((PrimaryNoNewArray1*)n); return;
            if (dynamic_cast<PrimaryNoNewArray2*>(n)) endVisit((PrimaryNoNewArray2*)n); return;
            if (dynamic_cast<PrimaryNoNewArray3*>(n)) endVisit((PrimaryNoNewArray3*)n); return;
            if (dynamic_cast<PrimaryNoNewArray4*>(n)) endVisit((PrimaryNoNewArray4*)n); return;
            if (dynamic_cast<Literal0*>(n)) endVisit((Literal0*)n); return;
            if (dynamic_cast<Literal1*>(n)) endVisit((Literal1*)n); return;
            if (dynamic_cast<Literal2*>(n)) endVisit((Literal2*)n); return;
            if (dynamic_cast<Literal3*>(n)) endVisit((Literal3*)n); return;
            if (dynamic_cast<Literal4*>(n)) endVisit((Literal4*)n); return;
            if (dynamic_cast<Literal5*>(n)) endVisit((Literal5*)n); return;
            if (dynamic_cast<Literal6*>(n)) endVisit((Literal6*)n); return;
            if (dynamic_cast<BooleanLiteral0*>(n)) endVisit((BooleanLiteral0*)n); return;
            if (dynamic_cast<BooleanLiteral1*>(n)) endVisit((BooleanLiteral1*)n); return;
            if (dynamic_cast<ClassInstanceCreationExpression0*>(n)) endVisit((ClassInstanceCreationExpression0*)n); return;
            if (dynamic_cast<ClassInstanceCreationExpression1*>(n)) endVisit((ClassInstanceCreationExpression1*)n); return;
            if (dynamic_cast<ArrayCreationExpression0*>(n)) endVisit((ArrayCreationExpression0*)n); return;
            if (dynamic_cast<ArrayCreationExpression1*>(n)) endVisit((ArrayCreationExpression1*)n); return;
            if (dynamic_cast<ArrayCreationExpression2*>(n)) endVisit((ArrayCreationExpression2*)n); return;
            if (dynamic_cast<ArrayCreationExpression3*>(n)) endVisit((ArrayCreationExpression3*)n); return;
            if (dynamic_cast<Dims0*>(n)) endVisit((Dims0*)n); return;
            if (dynamic_cast<Dims1*>(n)) endVisit((Dims1*)n); return;
            if (dynamic_cast<FieldAccess0*>(n)) endVisit((FieldAccess0*)n); return;
            if (dynamic_cast<FieldAccess1*>(n)) endVisit((FieldAccess1*)n); return;
            if (dynamic_cast<FieldAccess2*>(n)) endVisit((FieldAccess2*)n); return;
            if (dynamic_cast<MethodInvocation0*>(n)) endVisit((MethodInvocation0*)n); return;
            if (dynamic_cast<MethodInvocation1*>(n)) endVisit((MethodInvocation1*)n); return;
            if (dynamic_cast<MethodInvocation2*>(n)) endVisit((MethodInvocation2*)n); return;
            if (dynamic_cast<MethodInvocation3*>(n)) endVisit((MethodInvocation3*)n); return;
            if (dynamic_cast<MethodInvocation4*>(n)) endVisit((MethodInvocation4*)n); return;
            if (dynamic_cast<ArrayAccess0*>(n)) endVisit((ArrayAccess0*)n); return;
            if (dynamic_cast<ArrayAccess1*>(n)) endVisit((ArrayAccess1*)n); return;
            if (dynamic_cast<UnaryExpression0*>(n)) endVisit((UnaryExpression0*)n); return;
            if (dynamic_cast<UnaryExpression1*>(n)) endVisit((UnaryExpression1*)n); return;
            if (dynamic_cast<UnaryExpressionNotPlusMinus0*>(n)) endVisit((UnaryExpressionNotPlusMinus0*)n); return;
            if (dynamic_cast<UnaryExpressionNotPlusMinus1*>(n)) endVisit((UnaryExpressionNotPlusMinus1*)n); return;
            if (dynamic_cast<CastExpression0*>(n)) endVisit((CastExpression0*)n); return;
            if (dynamic_cast<CastExpression1*>(n)) endVisit((CastExpression1*)n); return;
            if (dynamic_cast<MultiplicativeExpression0*>(n)) endVisit((MultiplicativeExpression0*)n); return;
            if (dynamic_cast<MultiplicativeExpression1*>(n)) endVisit((MultiplicativeExpression1*)n); return;
            if (dynamic_cast<MultiplicativeExpression2*>(n)) endVisit((MultiplicativeExpression2*)n); return;
            if (dynamic_cast<AdditiveExpression0*>(n)) endVisit((AdditiveExpression0*)n); return;
            if (dynamic_cast<AdditiveExpression1*>(n)) endVisit((AdditiveExpression1*)n); return;
            if (dynamic_cast<ShiftExpression0*>(n)) endVisit((ShiftExpression0*)n); return;
            if (dynamic_cast<ShiftExpression1*>(n)) endVisit((ShiftExpression1*)n); return;
            if (dynamic_cast<ShiftExpression2*>(n)) endVisit((ShiftExpression2*)n); return;
            if (dynamic_cast<RelationalExpression0*>(n)) endVisit((RelationalExpression0*)n); return;
            if (dynamic_cast<RelationalExpression1*>(n)) endVisit((RelationalExpression1*)n); return;
            if (dynamic_cast<RelationalExpression2*>(n)) endVisit((RelationalExpression2*)n); return;
            if (dynamic_cast<RelationalExpression3*>(n)) endVisit((RelationalExpression3*)n); return;
            if (dynamic_cast<RelationalExpression4*>(n)) endVisit((RelationalExpression4*)n); return;
            if (dynamic_cast<EqualityExpression0*>(n)) endVisit((EqualityExpression0*)n); return;
            if (dynamic_cast<EqualityExpression1*>(n)) endVisit((EqualityExpression1*)n); return;
            if (dynamic_cast<AssignmentOperator0*>(n)) endVisit((AssignmentOperator0*)n); return;
            if (dynamic_cast<AssignmentOperator1*>(n)) endVisit((AssignmentOperator1*)n); return;
            if (dynamic_cast<AssignmentOperator2*>(n)) endVisit((AssignmentOperator2*)n); return;
            if (dynamic_cast<AssignmentOperator3*>(n)) endVisit((AssignmentOperator3*)n); return;
            if (dynamic_cast<AssignmentOperator4*>(n)) endVisit((AssignmentOperator4*)n); return;
            if (dynamic_cast<AssignmentOperator5*>(n)) endVisit((AssignmentOperator5*)n); return;
            if (dynamic_cast<AssignmentOperator6*>(n)) endVisit((AssignmentOperator6*)n); return;
            if (dynamic_cast<AssignmentOperator7*>(n)) endVisit((AssignmentOperator7*)n); return;
            if (dynamic_cast<AssignmentOperator8*>(n)) endVisit((AssignmentOperator8*)n); return;
            if (dynamic_cast<AssignmentOperator9*>(n)) endVisit((AssignmentOperator9*)n); return;
            if (dynamic_cast<AssignmentOperator10*>(n)) endVisit((AssignmentOperator10*)n); return;
            if (dynamic_cast<AssignmentOperator11*>(n)) endVisit((AssignmentOperator11*)n); return;
            throw UnsupportedOperationException("visit(" + n->to_utf8_string() + ")");
        }
    };

    //#line 351 "btParserTemplateF.gi


    void ruleAction(int ruleNumber)
    {
        switch (ruleNumber)
        {

            //
            // Rule 3:  identifier ::= IDENTIFIER
            //
        case 3: {
            //#line 184 "GJavaParser.g"
            setResult(
                //#line 184 GJavaParser.g
                ast_pool.data.Next() = new identifier(this, getRhsIToken(1))
                //#line 184 GJavaParser.g
            );
            break;
        }
              //
              // Rule 4:  LPGUserAction ::= BlockStatementsopt
              //
        case 4:
            break;
            //
            // Rule 5:  LPGUserAction ::= $BeginAction BlockStatementsopt $EndAction
            //
        case 5: {
            //#line 194 "GJavaParser.g"
            setResult(
                //#line 194 GJavaParser.g
                ast_pool.data.Next() = new LPGUserAction0(getLeftIToken(), getRightIToken(),
                    //#line 194 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 194 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 194 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 194 GJavaParser.g
            );
            break;
        }
              //
              // Rule 6:  LPGUserAction ::= $BeginJava BlockStatementsopt $EndJava
              //
        case 6: {
            //#line 195 "GJavaParser.g"
            setResult(
                //#line 195 GJavaParser.g
                ast_pool.data.Next() = new LPGUserAction1(getLeftIToken(), getRightIToken(),
                    //#line 195 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 195 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 195 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 195 GJavaParser.g
            );
            break;
        }
              //
              // Rule 7:  LPGUserAction ::= $NoAction
              //
        case 7: {
            //#line 196 "GJavaParser.g"
            setResult(
                //#line 196 GJavaParser.g
                ast_pool.data.Next() = new LPGUserAction2(getRhsIToken(1))
                //#line 196 GJavaParser.g
            );
            break;
        }
              //
              // Rule 8:  LPGUserAction ::= $NullAction
              //
        case 8: {
            //#line 197 "GJavaParser.g"
            setResult(
                //#line 197 GJavaParser.g
                ast_pool.data.Next() = new LPGUserAction3(getRhsIToken(1))
                //#line 197 GJavaParser.g
            );
            break;
        }
              //
              // Rule 9:  LPGUserAction ::= $BadAction
              //
        case 9: {
            //#line 198 "GJavaParser.g"
            setResult(
                //#line 198 GJavaParser.g
                ast_pool.data.Next() = new LPGUserAction4(getRhsIToken(1))
                //#line 198 GJavaParser.g
            );
            break;
        }
              //
              // Rule 10:  Type ::= PrimitiveType
              //
        case 10:
            break;
            //
            // Rule 11:  Type ::= ReferenceType
            //
        case 11:
            break;
            //
            // Rule 12:  PrimitiveType ::= NumericType
            //
        case 12:
            break;
            //
            // Rule 13:  PrimitiveType ::= boolean
            //
        case 13: {
            //#line 207 "GJavaParser.g"
            setResult(
                //#line 207 GJavaParser.g
                ast_pool.data.Next() = new PrimitiveType(getRhsIToken(1))
                //#line 207 GJavaParser.g
            );
            break;
        }
               //
               // Rule 14:  NumericType ::= IntegralType
               //
        case 14:
            break;
            //
            // Rule 15:  NumericType ::= FloatingPointType
            //
        case 15:
            break;
            //
            // Rule 16:  IntegralType ::= byte
            //
        case 16: {
            //#line 212 "GJavaParser.g"
            setResult(
                //#line 212 GJavaParser.g
                ast_pool.data.Next() = new IntegralType0(getRhsIToken(1))
                //#line 212 GJavaParser.g
            );
            break;
        }
               //
               // Rule 17:  IntegralType ::= short
               //
        case 17: {
            //#line 213 "GJavaParser.g"
            setResult(
                //#line 213 GJavaParser.g
                ast_pool.data.Next() = new IntegralType1(getRhsIToken(1))
                //#line 213 GJavaParser.g
            );
            break;
        }
               //
               // Rule 18:  IntegralType ::= int
               //
        case 18: {
            //#line 214 "GJavaParser.g"
            setResult(
                //#line 214 GJavaParser.g
                ast_pool.data.Next() = new IntegralType2(getRhsIToken(1))
                //#line 214 GJavaParser.g
            );
            break;
        }
               //
               // Rule 19:  IntegralType ::= long
               //
        case 19: {
            //#line 215 "GJavaParser.g"
            setResult(
                //#line 215 GJavaParser.g
                ast_pool.data.Next() = new IntegralType3(getRhsIToken(1))
                //#line 215 GJavaParser.g
            );
            break;
        }
               //
               // Rule 20:  IntegralType ::= char
               //
        case 20: {
            //#line 216 "GJavaParser.g"
            setResult(
                //#line 216 GJavaParser.g
                ast_pool.data.Next() = new IntegralType4(getRhsIToken(1))
                //#line 216 GJavaParser.g
            );
            break;
        }
               //
               // Rule 21:  FloatingPointType ::= float
               //
        case 21: {
            //#line 218 "GJavaParser.g"
            setResult(
                //#line 218 GJavaParser.g
                ast_pool.data.Next() = new FloatingPointType0(getRhsIToken(1))
                //#line 218 GJavaParser.g
            );
            break;
        }
               //
               // Rule 22:  FloatingPointType ::= double
               //
        case 22: {
            //#line 219 "GJavaParser.g"
            setResult(
                //#line 219 GJavaParser.g
                ast_pool.data.Next() = new FloatingPointType1(getRhsIToken(1))
                //#line 219 GJavaParser.g
            );
            break;
        }
               //
               // Rule 23:  ReferenceType ::= ClassOrInterfaceType
               //
        case 23:
            break;
            //
            // Rule 24:  ReferenceType ::= TypeVariable
            //
        case 24:
            break;
            //
            // Rule 25:  ReferenceType ::= ArrayType
            //
        case 25:
            break;
            //
            // Rule 26:  ClassOrInterfaceType ::= ClassType
            //
        case 26:
            break;
            //
            // Rule 27:  ClassType ::= TypeName TypeArgumentsopt
            //
        case 27: {
            //#line 231 "GJavaParser.g"
            setResult(
                //#line 231 GJavaParser.g
                ast_pool.data.Next() = new ClassType(getLeftIToken(), getRightIToken(),
                    //#line 231 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 231 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 231 GJavaParser.g
            );
            break;
        }
               //
               // Rule 28:  InterfaceType ::= TypeName TypeArgumentsopt
               //
        case 28: {
            //#line 233 "GJavaParser.g"
            setResult(
                //#line 233 GJavaParser.g
                ast_pool.data.Next() = new InterfaceType(getLeftIToken(), getRightIToken(),
                    //#line 233 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 233 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 233 GJavaParser.g
            );
            break;
        }
               //
               // Rule 29:  TypeName ::= identifier
               //
        case 29:
            break;
            //
            // Rule 30:  TypeName ::= TypeName . identifier
            //
        case 30: {
            //#line 236 "GJavaParser.g"
            setResult(
                //#line 236 GJavaParser.g
                ast_pool.data.Next() = new TypeName(getLeftIToken(), getRightIToken(),
                    //#line 236 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 236 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 236 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 236 GJavaParser.g
            );
            break;
        }
               //
               // Rule 31:  ClassName ::= TypeName
               //
        case 31:
            break;
            //
            // Rule 32:  TypeVariable ::= identifier
            //
        case 32:
            break;
            //
            // Rule 33:  ArrayType ::= Type [ ]
            //
        case 33: {
            //#line 242 "GJavaParser.g"
            setResult(
                //#line 242 GJavaParser.g
                ast_pool.data.Next() = new ArrayType(getLeftIToken(), getRightIToken(),
                    //#line 242 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 242 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 242 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 242 GJavaParser.g
            );
            break;
        }
               //
               // Rule 34:  TypeParameter ::= TypeVariable TypeBoundopt
               //
        case 34: {
            //#line 244 "GJavaParser.g"
            setResult(
                //#line 244 GJavaParser.g
                ast_pool.data.Next() = new TypeParameter(getLeftIToken(), getRightIToken(),
                    //#line 244 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 244 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 244 GJavaParser.g
            );
            break;
        }
               //
               // Rule 35:  TypeBound ::= extends ClassOrInterfaceType AdditionalBoundListopt
               //
        case 35: {
            //#line 246 "GJavaParser.g"
            setResult(
                //#line 246 GJavaParser.g
                ast_pool.data.Next() = new TypeBound(getLeftIToken(), getRightIToken(),
                    //#line 246 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 246 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 246 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 246 GJavaParser.g
            );
            break;
        }
               //
               // Rule 36:  AdditionalBoundList ::= AdditionalBound
               //
        case 36:
            break;
            //
            // Rule 37:  AdditionalBoundList ::= AdditionalBoundList AdditionalBound
            //
        case 37: {
            //#line 249 "GJavaParser.g"
            setResult(
                //#line 249 GJavaParser.g
                ast_pool.data.Next() = new AdditionalBoundList(getLeftIToken(), getRightIToken(),
                    //#line 249 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 249 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 249 GJavaParser.g
            );
            break;
        }
               //
               // Rule 38:  AdditionalBound ::= & InterfaceType
               //
        case 38: {
            //#line 251 "GJavaParser.g"
            setResult(
                //#line 251 GJavaParser.g
                ast_pool.data.Next() = new AdditionalBound(getLeftIToken(), getRightIToken(),
                    //#line 251 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 251 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 251 GJavaParser.g
            );
            break;
        }
               //
               // Rule 39:  TypeArguments ::= < ActualTypeArgumentList >
               //
        case 39: {
            //#line 253 "GJavaParser.g"
            setResult(
                //#line 253 GJavaParser.g
                ast_pool.data.Next() = new TypeArguments(getLeftIToken(), getRightIToken(),
                    //#line 253 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 253 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 253 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 253 GJavaParser.g
            );
            break;
        }
               //
               // Rule 40:  ActualTypeArgumentList ::= ActualTypeArgument
               //
        case 40:
            break;
            //
            // Rule 41:  ActualTypeArgumentList ::= ActualTypeArgumentList , ActualTypeArgument
            //
        case 41: {
            //#line 256 "GJavaParser.g"
            setResult(
                //#line 256 GJavaParser.g
                ast_pool.data.Next() = new ActualTypeArgumentList(getLeftIToken(), getRightIToken(),
                    //#line 256 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 256 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 256 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 256 GJavaParser.g
            );
            break;
        }
               //
               // Rule 42:  ActualTypeArgument ::= ReferenceType
               //
        case 42:
            break;
            //
            // Rule 43:  ActualTypeArgument ::= Wildcard
            //
        case 43:
            break;
            //
            // Rule 44:  Wildcard ::= ? WildcardBoundsOpt
            //
        case 44: {
            //#line 261 "GJavaParser.g"
            setResult(
                //#line 261 GJavaParser.g
                ast_pool.data.Next() = new Wildcard(getLeftIToken(), getRightIToken(),
                    //#line 261 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 261 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 261 GJavaParser.g
            );
            break;
        }
               //
               // Rule 45:  WildcardBounds ::= extends ReferenceType
               //
        case 45: {
            //#line 263 "GJavaParser.g"
            setResult(
                //#line 263 GJavaParser.g
                ast_pool.data.Next() = new WildcardBounds0(getLeftIToken(), getRightIToken(),
                    //#line 263 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 263 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 263 GJavaParser.g
            );
            break;
        }
               //
               // Rule 46:  WildcardBounds ::= super ReferenceType
               //
        case 46: {
            //#line 264 "GJavaParser.g"
            setResult(
                //#line 264 GJavaParser.g
                ast_pool.data.Next() = new WildcardBounds1(getLeftIToken(), getRightIToken(),
                    //#line 264 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 264 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 264 GJavaParser.g
            );
            break;
        }
               //
               // Rule 47:  PackageName ::= identifier
               //
        case 47:
            break;
            //
            // Rule 48:  PackageName ::= PackageName . identifier
            //
        case 48: {
            //#line 271 "GJavaParser.g"
            setResult(
                //#line 271 GJavaParser.g
                ast_pool.data.Next() = new PackageName(getLeftIToken(), getRightIToken(),
                    //#line 271 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 271 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 271 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 271 GJavaParser.g
            );
            break;
        }
               //
               // Rule 49:  ExpressionName ::= identifier
               //
        case 49:
            break;
            //
            // Rule 50:  ExpressionName ::= AmbiguousName . identifier
            //
        case 50: {
            //#line 280 "GJavaParser.g"
            setResult(
                //#line 280 GJavaParser.g
                ast_pool.data.Next() = new ExpressionName(getLeftIToken(), getRightIToken(),
                    //#line 280 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 280 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 280 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 280 GJavaParser.g
            );
            break;
        }
               //
               // Rule 51:  MethodName ::= identifier
               //
        case 51:
            break;
            //
            // Rule 52:  MethodName ::= AmbiguousName . identifier
            //
        case 52: {
            //#line 283 "GJavaParser.g"
            setResult(
                //#line 283 GJavaParser.g
                ast_pool.data.Next() = new MethodName(getLeftIToken(), getRightIToken(),
                    //#line 283 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 283 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 283 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 283 GJavaParser.g
            );
            break;
        }
               //
               // Rule 53:  PackageOrTypeName ::= identifier
               //
        case 53:
            break;
            //
            // Rule 54:  PackageOrTypeName ::= PackageOrTypeName . identifier
            //
        case 54: {
            //#line 286 "GJavaParser.g"
            setResult(
                //#line 286 GJavaParser.g
                ast_pool.data.Next() = new PackageOrTypeName(getLeftIToken(), getRightIToken(),
                    //#line 286 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 286 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 286 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 286 GJavaParser.g
            );
            break;
        }
               //
               // Rule 55:  AmbiguousName ::= identifier
               //
        case 55:
            break;
            //
            // Rule 56:  AmbiguousName ::= AmbiguousName . identifier
            //
        case 56: {
            //#line 289 "GJavaParser.g"
            setResult(
                //#line 289 GJavaParser.g
                ast_pool.data.Next() = new AmbiguousName(getLeftIToken(), getRightIToken(),
                    //#line 289 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 289 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 289 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 289 GJavaParser.g
            );
            break;
        }
               //
               // Rule 57:  CompilationUnit ::= PackageDeclarationopt ImportDeclarationsopt TypeDeclarationsopt
               //
        case 57: {
            //#line 293 "GJavaParser.g"
            setResult(
                //#line 293 GJavaParser.g
                ast_pool.data.Next() = new CompilationUnit(getLeftIToken(), getRightIToken(),
                    //#line 293 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 293 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 293 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 293 GJavaParser.g
            );
            break;
        }
               //
               // Rule 58:  ImportDeclarations ::= ImportDeclaration
               //
        case 58:
            break;
            //
            // Rule 59:  ImportDeclarations ::= ImportDeclarations ImportDeclaration
            //
        case 59: {
            //#line 296 "GJavaParser.g"
            setResult(
                //#line 296 GJavaParser.g
                ast_pool.data.Next() = new ImportDeclarations(getLeftIToken(), getRightIToken(),
                    //#line 296 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 296 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 296 GJavaParser.g
            );
            break;
        }
               //
               // Rule 60:  TypeDeclarations ::= TypeDeclaration
               //
        case 60:
            break;
            //
            // Rule 61:  TypeDeclarations ::= TypeDeclarations TypeDeclaration
            //
        case 61: {
            //#line 299 "GJavaParser.g"
            setResult(
                //#line 299 GJavaParser.g
                ast_pool.data.Next() = new TypeDeclarations(getLeftIToken(), getRightIToken(),
                    //#line 299 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 299 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 299 GJavaParser.g
            );
            break;
        }
               //
               // Rule 62:  PackageDeclaration ::= Annotationsopt package PackageName ;
               //
        case 62: {
            //#line 301 "GJavaParser.g"
            setResult(
                //#line 301 GJavaParser.g
                ast_pool.data.Next() = new PackageDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 301 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 301 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 301 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 301 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 301 GJavaParser.g
            );
            break;
        }
               //
               // Rule 63:  ImportDeclaration ::= SingleTypeImportDeclaration
               //
        case 63:
            break;
            //
            // Rule 64:  ImportDeclaration ::= TypeImportOnDemandDeclaration
            //
        case 64:
            break;
            //
            // Rule 65:  ImportDeclaration ::= SingleStaticImportDeclaration
            //
        case 65:
            break;
            //
            // Rule 66:  ImportDeclaration ::= StaticImportOnDemandDeclaration
            //
        case 66:
            break;
            //
            // Rule 67:  SingleTypeImportDeclaration ::= import TypeName ;
            //
        case 67: {
            //#line 308 "GJavaParser.g"
            setResult(
                //#line 308 GJavaParser.g
                ast_pool.data.Next() = new SingleTypeImportDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 308 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 308 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 308 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 308 GJavaParser.g
            );
            break;
        }
               //
               // Rule 68:  TypeImportOnDemandDeclaration ::= import PackageOrTypeName . * ;
               //
        case 68: {
            //#line 310 "GJavaParser.g"
            setResult(
                //#line 310 GJavaParser.g
                ast_pool.data.Next() = new TypeImportOnDemandDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 310 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 310 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 310 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 310 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 310 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)))
                //#line 310 GJavaParser.g
            );
            break;
        }
               //
               // Rule 69:  SingleStaticImportDeclaration ::= import static TypeName . identifier ;
               //
        case 69: {
            //#line 312 "GJavaParser.g"
            setResult(
                //#line 312 GJavaParser.g
                ast_pool.data.Next() = new SingleStaticImportDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 312 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 312 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 312 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 312 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 312 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 312 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)))
                //#line 312 GJavaParser.g
            );
            break;
        }
               //
               // Rule 70:  StaticImportOnDemandDeclaration ::= import static TypeName . * ;
               //
        case 70: {
            //#line 314 "GJavaParser.g"
            setResult(
                //#line 314 GJavaParser.g
                ast_pool.data.Next() = new StaticImportOnDemandDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 314 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 314 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 314 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 314 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 314 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 314 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)))
                //#line 314 GJavaParser.g
            );
            break;
        }
               //
               // Rule 71:  TypeDeclaration ::= ClassDeclaration
               //
        case 71:
            break;
            //
            // Rule 72:  TypeDeclaration ::= InterfaceDeclaration
            //
        case 72:
            break;
            //
            // Rule 73:  TypeDeclaration ::= ;
            //
        case 73: {
            //#line 318 "GJavaParser.g"
            setResult(
                //#line 318 GJavaParser.g
                ast_pool.data.Next() = new TypeDeclaration(getRhsIToken(1))
                //#line 318 GJavaParser.g
            );
            break;
        }
               //
               // Rule 74:  ClassDeclaration ::= NormalClassDeclaration
               //
        case 74:
            break;
            //
            // Rule 75:  ClassDeclaration ::= EnumDeclaration
            //
        case 75:
            break;
            //
            // Rule 76:  NormalClassDeclaration ::= ClassModifiersopt class identifier TypeParametersopt Superopt Interfacesopt ClassBody
            //
        case 76: {
            //#line 325 "GJavaParser.g"
            setResult(
                //#line 325 GJavaParser.g
                ast_pool.data.Next() = new NormalClassDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 325 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 325 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 325 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 325 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 325 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 325 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 325 GJavaParser.g
                    (IAst*)getRhsSym(7))
                //#line 325 GJavaParser.g
            );
            break;
        }
               //
               // Rule 77:  ClassModifiers ::= ClassModifier
               //
        case 77:
            break;
            //
            // Rule 78:  ClassModifiers ::= ClassModifiers ClassModifier
            //
        case 78: {
            //#line 328 "GJavaParser.g"
            setResult(
                //#line 328 GJavaParser.g
                ast_pool.data.Next() = new ClassModifiers(getLeftIToken(), getRightIToken(),
                    //#line 328 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 328 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 328 GJavaParser.g
            );
            break;
        }
               //
               // Rule 79:  ClassModifier ::= Annotation
               //
        case 79:
            break;
            //
            // Rule 80:  ClassModifier ::= public
            //
        case 80: {
            //#line 331 "GJavaParser.g"
            setResult(
                //#line 331 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier0(getRhsIToken(1))
                //#line 331 GJavaParser.g
            );
            break;
        }
               //
               // Rule 81:  ClassModifier ::= protected
               //
        case 81: {
            //#line 332 "GJavaParser.g"
            setResult(
                //#line 332 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier1(getRhsIToken(1))
                //#line 332 GJavaParser.g
            );
            break;
        }
               //
               // Rule 82:  ClassModifier ::= private
               //
        case 82: {
            //#line 333 "GJavaParser.g"
            setResult(
                //#line 333 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier2(getRhsIToken(1))
                //#line 333 GJavaParser.g
            );
            break;
        }
               //
               // Rule 83:  ClassModifier ::= abstract
               //
        case 83: {
            //#line 334 "GJavaParser.g"
            setResult(
                //#line 334 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier3(getRhsIToken(1))
                //#line 334 GJavaParser.g
            );
            break;
        }
               //
               // Rule 84:  ClassModifier ::= static
               //
        case 84: {
            //#line 335 "GJavaParser.g"
            setResult(
                //#line 335 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier4(getRhsIToken(1))
                //#line 335 GJavaParser.g
            );
            break;
        }
               //
               // Rule 85:  ClassModifier ::= final
               //
        case 85: {
            //#line 336 "GJavaParser.g"
            setResult(
                //#line 336 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier5(getRhsIToken(1))
                //#line 336 GJavaParser.g
            );
            break;
        }
               //
               // Rule 86:  ClassModifier ::= strictfp
               //
        case 86: {
            //#line 337 "GJavaParser.g"
            setResult(
                //#line 337 GJavaParser.g
                ast_pool.data.Next() = new ClassModifier6(getRhsIToken(1))
                //#line 337 GJavaParser.g
            );
            break;
        }
               //
               // Rule 87:  TypeParameters ::= < TypeParameterList >
               //
        case 87: {
            //#line 339 "GJavaParser.g"
            setResult(
                //#line 339 GJavaParser.g
                ast_pool.data.Next() = new TypeParameters(getLeftIToken(), getRightIToken(),
                    //#line 339 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 339 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 339 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 339 GJavaParser.g
            );
            break;
        }
               //
               // Rule 88:  TypeParameterList ::= TypeParameter
               //
        case 88:
            break;
            //
            // Rule 89:  TypeParameterList ::= TypeParameterList , TypeParameter
            //
        case 89: {
            //#line 342 "GJavaParser.g"
            setResult(
                //#line 342 GJavaParser.g
                ast_pool.data.Next() = new TypeParameterList(getLeftIToken(), getRightIToken(),
                    //#line 342 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 342 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 342 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 342 GJavaParser.g
            );
            break;
        }
               //
               // Rule 90:  Super ::= extends ClassType
               //
        case 90: {
            //#line 344 "GJavaParser.g"
            setResult(
                //#line 344 GJavaParser.g
                ast_pool.data.Next() = new Super(getLeftIToken(), getRightIToken(),
                    //#line 344 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 344 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 344 GJavaParser.g
            );
            break;
        }
               //
               // Rule 91:  Interfaces ::= implements InterfaceTypeList
               //
        case 91: {
            //#line 351 "GJavaParser.g"
            setResult(
                //#line 351 GJavaParser.g
                ast_pool.data.Next() = new Interfaces(getLeftIToken(), getRightIToken(),
                    //#line 351 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 351 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 351 GJavaParser.g
            );
            break;
        }
               //
               // Rule 92:  InterfaceTypeList ::= InterfaceType
               //
        case 92:
            break;
            //
            // Rule 93:  InterfaceTypeList ::= InterfaceTypeList , InterfaceType
            //
        case 93: {
            //#line 354 "GJavaParser.g"
            setResult(
                //#line 354 GJavaParser.g
                ast_pool.data.Next() = new InterfaceTypeList(getLeftIToken(), getRightIToken(),
                    //#line 354 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 354 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 354 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 354 GJavaParser.g
            );
            break;
        }
               //
               // Rule 94:  ClassBody ::= { ClassBodyDeclarationsopt }
               //
        case 94: {
            //#line 361 "GJavaParser.g"
            setResult(
                //#line 361 GJavaParser.g
                ast_pool.data.Next() = new ClassBody(getLeftIToken(), getRightIToken(),
                    //#line 361 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 361 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 361 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 361 GJavaParser.g
            );
            break;
        }
               //
               // Rule 95:  ClassBodyDeclarations ::= ClassBodyDeclaration
               //
        case 95:
            break;
            //
            // Rule 96:  ClassBodyDeclarations ::= ClassBodyDeclarations ClassBodyDeclaration
            //
        case 96: {
            //#line 364 "GJavaParser.g"
            setResult(
                //#line 364 GJavaParser.g
                ast_pool.data.Next() = new ClassBodyDeclarations(getLeftIToken(), getRightIToken(),
                    //#line 364 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 364 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 364 GJavaParser.g
            );
            break;
        }
               //
               // Rule 97:  ClassBodyDeclaration ::= ClassMemberDeclaration
               //
        case 97:
            break;
            //
            // Rule 98:  ClassBodyDeclaration ::= InstanceInitializer
            //
        case 98:
            break;
            //
            // Rule 99:  ClassBodyDeclaration ::= StaticInitializer
            //
        case 99:
            break;
            //
            // Rule 100:  ClassBodyDeclaration ::= ConstructorDeclaration
            //
        case 100:
            break;
            //
            // Rule 101:  ClassMemberDeclaration ::= FieldDeclaration
            //
        case 101:
            break;
            //
            // Rule 102:  ClassMemberDeclaration ::= MethodDeclaration
            //
        case 102:
            break;
            //
            // Rule 103:  ClassMemberDeclaration ::= ClassDeclaration
            //
        case 103:
            break;
            //
            // Rule 104:  ClassMemberDeclaration ::= InterfaceDeclaration
            //
        case 104:
            break;
            //
            // Rule 105:  ClassMemberDeclaration ::= ;
            //
        case 105: {
            //#line 375 "GJavaParser.g"
            setResult(
                //#line 375 GJavaParser.g
                ast_pool.data.Next() = new ClassMemberDeclaration(getRhsIToken(1))
                //#line 375 GJavaParser.g
            );
            break;
        }
                //
                // Rule 106:  FieldDeclaration ::= FieldModifiersopt Type VariableDeclarators ;
                //
        case 106: {
            //#line 377 "GJavaParser.g"
            setResult(
                //#line 377 GJavaParser.g
                ast_pool.data.Next() = new FieldDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 377 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 377 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 377 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 377 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 377 GJavaParser.g
            );
            break;
        }
                //
                // Rule 107:  VariableDeclarators ::= VariableDeclarator
                //
        case 107:
            break;
            //
            // Rule 108:  VariableDeclarators ::= VariableDeclarators , VariableDeclarator
            //
        case 108: {
            //#line 380 "GJavaParser.g"
            setResult(
                //#line 380 GJavaParser.g
                ast_pool.data.Next() = new VariableDeclarators(getLeftIToken(), getRightIToken(),
                    //#line 380 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 380 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 380 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 380 GJavaParser.g
            );
            break;
        }
                //
                // Rule 109:  VariableDeclarator ::= VariableDeclaratorId
                //
        case 109:
            break;
            //
            // Rule 110:  VariableDeclarator ::= VariableDeclaratorId = VariableInitializer
            //
        case 110: {
            //#line 383 "GJavaParser.g"
            setResult(
                //#line 383 GJavaParser.g
                ast_pool.data.Next() = new VariableDeclarator(getLeftIToken(), getRightIToken(),
                    //#line 383 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 383 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 383 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 383 GJavaParser.g
            );
            break;
        }
                //
                // Rule 111:  VariableDeclaratorId ::= identifier
                //
        case 111:
            break;
            //
            // Rule 112:  VariableDeclaratorId ::= VariableDeclaratorId [ ]
            //
        case 112: {
            //#line 386 "GJavaParser.g"
            setResult(
                //#line 386 GJavaParser.g
                ast_pool.data.Next() = new VariableDeclaratorId(getLeftIToken(), getRightIToken(),
                    //#line 386 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 386 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 386 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 386 GJavaParser.g
            );
            break;
        }
                //
                // Rule 113:  VariableInitializer ::= Expression
                //
        case 113:
            break;
            //
            // Rule 114:  VariableInitializer ::= ArrayInitializer
            //
        case 114:
            break;
            //
            // Rule 115:  FieldModifiers ::= FieldModifier
            //
        case 115:
            break;
            //
            // Rule 116:  FieldModifiers ::= FieldModifiers FieldModifier
            //
        case 116: {
            //#line 392 "GJavaParser.g"
            setResult(
                //#line 392 GJavaParser.g
                ast_pool.data.Next() = new FieldModifiers(getLeftIToken(), getRightIToken(),
                    //#line 392 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 392 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 392 GJavaParser.g
            );
            break;
        }
                //
                // Rule 117:  FieldModifier ::= Annotation
                //
        case 117:
            break;
            //
            // Rule 118:  FieldModifier ::= public
            //
        case 118: {
            //#line 395 "GJavaParser.g"
            setResult(
                //#line 395 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier0(getRhsIToken(1))
                //#line 395 GJavaParser.g
            );
            break;
        }
                //
                // Rule 119:  FieldModifier ::= protected
                //
        case 119: {
            //#line 396 "GJavaParser.g"
            setResult(
                //#line 396 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier1(getRhsIToken(1))
                //#line 396 GJavaParser.g
            );
            break;
        }
                //
                // Rule 120:  FieldModifier ::= private
                //
        case 120: {
            //#line 397 "GJavaParser.g"
            setResult(
                //#line 397 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier2(getRhsIToken(1))
                //#line 397 GJavaParser.g
            );
            break;
        }
                //
                // Rule 121:  FieldModifier ::= static
                //
        case 121: {
            //#line 398 "GJavaParser.g"
            setResult(
                //#line 398 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier3(getRhsIToken(1))
                //#line 398 GJavaParser.g
            );
            break;
        }
                //
                // Rule 122:  FieldModifier ::= final
                //
        case 122: {
            //#line 399 "GJavaParser.g"
            setResult(
                //#line 399 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier4(getRhsIToken(1))
                //#line 399 GJavaParser.g
            );
            break;
        }
                //
                // Rule 123:  FieldModifier ::= transient
                //
        case 123: {
            //#line 400 "GJavaParser.g"
            setResult(
                //#line 400 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier5(getRhsIToken(1))
                //#line 400 GJavaParser.g
            );
            break;
        }
                //
                // Rule 124:  FieldModifier ::= volatile
                //
        case 124: {
            //#line 401 "GJavaParser.g"
            setResult(
                //#line 401 GJavaParser.g
                ast_pool.data.Next() = new FieldModifier6(getRhsIToken(1))
                //#line 401 GJavaParser.g
            );
            break;
        }
                //
                // Rule 125:  MethodDeclaration ::= MethodHeader MethodBody
                //
        case 125: {
            //#line 403 "GJavaParser.g"
            setResult(
                //#line 403 GJavaParser.g
                ast_pool.data.Next() = new MethodDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 403 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 403 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 403 GJavaParser.g
            );
            break;
        }
                //
                // Rule 126:  MethodHeader ::= MethodModifiersopt TypeParametersopt ResultType MethodDeclarator Throwsopt
                //
        case 126: {
            //#line 405 "GJavaParser.g"
            setResult(
                //#line 405 GJavaParser.g
                ast_pool.data.Next() = new MethodHeader(getLeftIToken(), getRightIToken(),
                    //#line 405 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 405 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 405 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 405 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 405 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 405 GJavaParser.g
            );
            break;
        }
                //
                // Rule 127:  ResultType ::= Type
                //
        case 127:
            break;
            //
            // Rule 128:  ResultType ::= void
            //
        case 128: {
            //#line 408 "GJavaParser.g"
            setResult(
                //#line 408 GJavaParser.g
                ast_pool.data.Next() = new ResultType(getRhsIToken(1))
                //#line 408 GJavaParser.g
            );
            break;
        }
                //
                // Rule 129:  MethodDeclarator ::= identifier ( FormalParameterListopt )
                //
        case 129: {
            //#line 410 "GJavaParser.g"
            setResult(
                //#line 410 GJavaParser.g
                ast_pool.data.Next() = new MethodDeclarator0(getLeftIToken(), getRightIToken(),
                    //#line 410 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 410 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 410 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 410 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 410 GJavaParser.g
            );
            break;
        }
                //
                // Rule 130:  MethodDeclarator ::= MethodDeclarator [ ]
                //
        case 130: {
            //#line 412 "GJavaParser.g"
            setResult(
                //#line 412 GJavaParser.g
                ast_pool.data.Next() = new MethodDeclarator1(getLeftIToken(), getRightIToken(),
                    //#line 412 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 412 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 412 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 412 GJavaParser.g
            );
            break;
        }
                //
                // Rule 131:  FormalParameterList ::= LastFormalParameter
                //
        case 131:
            break;
            //
            // Rule 132:  FormalParameterList ::= FormalParameters , LastFormalParameter
            //
        case 132: {
            //#line 415 "GJavaParser.g"
            setResult(
                //#line 415 GJavaParser.g
                ast_pool.data.Next() = new FormalParameterList(getLeftIToken(), getRightIToken(),
                    //#line 415 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 415 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 415 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 415 GJavaParser.g
            );
            break;
        }
                //
                // Rule 133:  FormalParameters ::= FormalParameter
                //
        case 133:
            break;
            //
            // Rule 134:  FormalParameters ::= FormalParameters , FormalParameter
            //
        case 134: {
            //#line 418 "GJavaParser.g"
            setResult(
                //#line 418 GJavaParser.g
                ast_pool.data.Next() = new FormalParameters(getLeftIToken(), getRightIToken(),
                    //#line 418 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 418 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 418 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 418 GJavaParser.g
            );
            break;
        }
                //
                // Rule 135:  FormalParameter ::= VariableModifiersopt Type VariableDeclaratorId
                //
        case 135: {
            //#line 420 "GJavaParser.g"
            setResult(
                //#line 420 GJavaParser.g
                ast_pool.data.Next() = new FormalParameter(getLeftIToken(), getRightIToken(),
                    //#line 420 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 420 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 420 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 420 GJavaParser.g
            );
            break;
        }
                //
                // Rule 136:  VariableModifiers ::= VariableModifier
                //
        case 136:
            break;
            //
            // Rule 137:  VariableModifiers ::= VariableModifiers VariableModifier
            //
        case 137: {
            //#line 423 "GJavaParser.g"
            setResult(
                //#line 423 GJavaParser.g
                ast_pool.data.Next() = new VariableModifiers(getLeftIToken(), getRightIToken(),
                    //#line 423 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 423 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 423 GJavaParser.g
            );
            break;
        }
                //
                // Rule 138:  VariableModifier ::= final
                //
        case 138: {
            //#line 425 "GJavaParser.g"
            setResult(
                //#line 425 GJavaParser.g
                ast_pool.data.Next() = new VariableModifier(getRhsIToken(1))
                //#line 425 GJavaParser.g
            );
            break;
        }
                //
                // Rule 139:  VariableModifier ::= Annotations
                //
        case 139:
            break;
            //
            // Rule 140:  LastFormalParameter ::= VariableModifiersopt Type ...opt VariableDeclaratorId
            //
        case 140: {
            //#line 428 "GJavaParser.g"
            setResult(
                //#line 428 GJavaParser.g
                ast_pool.data.Next() = new LastFormalParameter(getLeftIToken(), getRightIToken(),
                    //#line 428 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 428 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 428 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 428 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 428 GJavaParser.g
            );
            break;
        }
                //
                // Rule 141:  MethodModifiers ::= MethodModifier
                //
        case 141:
            break;
            //
            // Rule 142:  MethodModifiers ::= MethodModifiers MethodModifier
            //
        case 142: {
            //#line 437 "GJavaParser.g"
            setResult(
                //#line 437 GJavaParser.g
                ast_pool.data.Next() = new MethodModifiers(getLeftIToken(), getRightIToken(),
                    //#line 437 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 437 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 437 GJavaParser.g
            );
            break;
        }
                //
                // Rule 143:  MethodModifier ::= Annotations
                //
        case 143:
            break;
            //
            // Rule 144:  MethodModifier ::= public
            //
        case 144: {
            //#line 440 "GJavaParser.g"
            setResult(
                //#line 440 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier0(getRhsIToken(1))
                //#line 440 GJavaParser.g
            );
            break;
        }
                //
                // Rule 145:  MethodModifier ::= protected
                //
        case 145: {
            //#line 441 "GJavaParser.g"
            setResult(
                //#line 441 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier1(getRhsIToken(1))
                //#line 441 GJavaParser.g
            );
            break;
        }
                //
                // Rule 146:  MethodModifier ::= private
                //
        case 146: {
            //#line 442 "GJavaParser.g"
            setResult(
                //#line 442 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier2(getRhsIToken(1))
                //#line 442 GJavaParser.g
            );
            break;
        }
                //
                // Rule 147:  MethodModifier ::= abstract
                //
        case 147: {
            //#line 443 "GJavaParser.g"
            setResult(
                //#line 443 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier3(getRhsIToken(1))
                //#line 443 GJavaParser.g
            );
            break;
        }
                //
                // Rule 148:  MethodModifier ::= static
                //
        case 148: {
            //#line 444 "GJavaParser.g"
            setResult(
                //#line 444 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier4(getRhsIToken(1))
                //#line 444 GJavaParser.g
            );
            break;
        }
                //
                // Rule 149:  MethodModifier ::= final
                //
        case 149: {
            //#line 445 "GJavaParser.g"
            setResult(
                //#line 445 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier5(getRhsIToken(1))
                //#line 445 GJavaParser.g
            );
            break;
        }
                //
                // Rule 150:  MethodModifier ::= synchronized
                //
        case 150: {
            //#line 446 "GJavaParser.g"
            setResult(
                //#line 446 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier6(getRhsIToken(1))
                //#line 446 GJavaParser.g
            );
            break;
        }
                //
                // Rule 151:  MethodModifier ::= native
                //
        case 151: {
            //#line 447 "GJavaParser.g"
            setResult(
                //#line 447 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier7(getRhsIToken(1))
                //#line 447 GJavaParser.g
            );
            break;
        }
                //
                // Rule 152:  MethodModifier ::= strictfp
                //
        case 152: {
            //#line 448 "GJavaParser.g"
            setResult(
                //#line 448 GJavaParser.g
                ast_pool.data.Next() = new MethodModifier8(getRhsIToken(1))
                //#line 448 GJavaParser.g
            );
            break;
        }
                //
                // Rule 153:  Throws ::= throws ExceptionTypeList
                //
        case 153: {
            //#line 450 "GJavaParser.g"
            setResult(
                //#line 450 GJavaParser.g
                ast_pool.data.Next() = new Throws(getLeftIToken(), getRightIToken(),
                    //#line 450 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 450 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 450 GJavaParser.g
            );
            break;
        }
                //
                // Rule 154:  ExceptionTypeList ::= ExceptionType
                //
        case 154:
            break;
            //
            // Rule 155:  ExceptionTypeList ::= ExceptionTypeList , ExceptionType
            //
        case 155: {
            //#line 453 "GJavaParser.g"
            setResult(
                //#line 453 GJavaParser.g
                ast_pool.data.Next() = new ExceptionTypeList(getLeftIToken(), getRightIToken(),
                    //#line 453 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 453 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 453 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 453 GJavaParser.g
            );
            break;
        }
                //
                // Rule 156:  ExceptionType ::= ClassType
                //
        case 156:
            break;
            //
            // Rule 157:  ExceptionType ::= TypeVariable
            //
        case 157:
            break;
            //
            // Rule 158:  MethodBody ::= Block
            //
        case 158:
            break;
            //
            // Rule 159:  MethodBody ::= ;
            //
        case 159: {
            //#line 459 "GJavaParser.g"
            setResult(
                //#line 459 GJavaParser.g
                ast_pool.data.Next() = new MethodBody(getRhsIToken(1))
                //#line 459 GJavaParser.g
            );
            break;
        }
                //
                // Rule 160:  InstanceInitializer ::= Block
                //
        case 160:
            break;
            //
            // Rule 161:  StaticInitializer ::= static Block
            //
        case 161: {
            //#line 463 "GJavaParser.g"
            setResult(
                //#line 463 GJavaParser.g
                ast_pool.data.Next() = new StaticInitializer(getLeftIToken(), getRightIToken(),
                    //#line 463 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 463 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 463 GJavaParser.g
            );
            break;
        }
                //
                // Rule 162:  ConstructorDeclaration ::= ConstructorModifiersopt ConstructorDeclarator Throwsopt ConstructorBody
                //
        case 162: {
            //#line 465 "GJavaParser.g"
            setResult(
                //#line 465 GJavaParser.g
                ast_pool.data.Next() = new ConstructorDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 465 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 465 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 465 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 465 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 465 GJavaParser.g
            );
            break;
        }
                //
                // Rule 163:  ConstructorDeclarator ::= TypeParametersopt SimpleTypeName ( FormalParameterListopt )
                //
        case 163: {
            //#line 467 "GJavaParser.g"
            setResult(
                //#line 467 GJavaParser.g
                ast_pool.data.Next() = new ConstructorDeclarator(getLeftIToken(), getRightIToken(),
                    //#line 467 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 467 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 467 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 467 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 467 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)))
                //#line 467 GJavaParser.g
            );
            break;
        }
                //
                // Rule 164:  SimpleTypeName ::= identifier
                //
        case 164:
            break;
            //
            // Rule 165:  ConstructorModifiers ::= ConstructorModifier
            //
        case 165:
            break;
            //
            // Rule 166:  ConstructorModifiers ::= ConstructorModifiers ConstructorModifier
            //
        case 166: {
            //#line 472 "GJavaParser.g"
            setResult(
                //#line 472 GJavaParser.g
                ast_pool.data.Next() = new ConstructorModifiers(getLeftIToken(), getRightIToken(),
                    //#line 472 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 472 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 472 GJavaParser.g
            );
            break;
        }
                //
                // Rule 167:  ConstructorModifier ::= Annotations
                //
        case 167:
            break;
            //
            // Rule 168:  ConstructorModifier ::= public
            //
        case 168: {
            //#line 475 "GJavaParser.g"
            setResult(
                //#line 475 GJavaParser.g
                ast_pool.data.Next() = new ConstructorModifier0(getRhsIToken(1))
                //#line 475 GJavaParser.g
            );
            break;
        }
                //
                // Rule 169:  ConstructorModifier ::= protected
                //
        case 169: {
            //#line 476 "GJavaParser.g"
            setResult(
                //#line 476 GJavaParser.g
                ast_pool.data.Next() = new ConstructorModifier1(getRhsIToken(1))
                //#line 476 GJavaParser.g
            );
            break;
        }
                //
                // Rule 170:  ConstructorModifier ::= private
                //
        case 170: {
            //#line 477 "GJavaParser.g"
            setResult(
                //#line 477 GJavaParser.g
                ast_pool.data.Next() = new ConstructorModifier2(getRhsIToken(1))
                //#line 477 GJavaParser.g
            );
            break;
        }
                //
                // Rule 171:  ConstructorBody ::= { ExplicitConstructorInvocationopt BlockStatementsopt }
                //
        case 171: {
            //#line 479 "GJavaParser.g"
            setResult(
                //#line 479 GJavaParser.g
                ast_pool.data.Next() = new ConstructorBody(getLeftIToken(), getRightIToken(),
                    //#line 479 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 479 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 479 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 479 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 479 GJavaParser.g
            );
            break;
        }
                //
                // Rule 172:  ExplicitConstructorInvocation ::= TypeArgumentsopt this ( ArgumentListopt ) ;
                //
        case 172: {
            //#line 481 "GJavaParser.g"
            setResult(
                //#line 481 GJavaParser.g
                ast_pool.data.Next() = new ExplicitConstructorInvocation0(getLeftIToken(), getRightIToken(),
                    //#line 481 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 481 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 481 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 481 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 481 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 481 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)))
                //#line 481 GJavaParser.g
            );
            break;
        }
                //
                // Rule 173:  ExplicitConstructorInvocation ::= TypeArgumentsopt super ( ArgumentListopt ) ;
                //
        case 173: {
            //#line 482 "GJavaParser.g"
            setResult(
                //#line 482 GJavaParser.g
                ast_pool.data.Next() = new ExplicitConstructorInvocation1(getLeftIToken(), getRightIToken(),
                    //#line 482 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 482 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 482 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 482 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 482 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 482 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)))
                //#line 482 GJavaParser.g
            );
            break;
        }
                //
                // Rule 174:  ExplicitConstructorInvocation ::= Primary . TypeArgumentsopt super ( ArgumentListopt ) ;
                //
        case 174: {
            //#line 483 "GJavaParser.g"
            setResult(
                //#line 483 GJavaParser.g
                ast_pool.data.Next() = new ExplicitConstructorInvocation2(getLeftIToken(), getRightIToken(),
                    //#line 483 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 483 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 483 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 483 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 483 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 483 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 483 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)),
                    //#line 483 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(8)))
                //#line 483 GJavaParser.g
            );
            break;
        }
                //
                // Rule 175:  EnumDeclaration ::= ClassModifiersopt enum identifier Interfacesopt EnumBody
                //
        case 175: {
            //#line 485 "GJavaParser.g"
            setResult(
                //#line 485 GJavaParser.g
                ast_pool.data.Next() = new EnumDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 485 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 485 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 485 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 485 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 485 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 485 GJavaParser.g
            );
            break;
        }
                //
                // Rule 176:  EnumBody ::= { EnumConstantsopt ,opt EnumBodyDeclarationsopt }
                //
        case 176: {
            //#line 487 "GJavaParser.g"
            setResult(
                //#line 487 GJavaParser.g
                ast_pool.data.Next() = new EnumBody(getLeftIToken(), getRightIToken(),
                    //#line 487 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 487 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 487 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 487 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 487 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)))
                //#line 487 GJavaParser.g
            );
            break;
        }
                //
                // Rule 177:  EnumConstants ::= EnumConstant
                //
        case 177:
            break;
            //
            // Rule 178:  EnumConstants ::= EnumConstants , EnumConstant
            //
        case 178: {
            //#line 490 "GJavaParser.g"
            setResult(
                //#line 490 GJavaParser.g
                ast_pool.data.Next() = new EnumConstants(getLeftIToken(), getRightIToken(),
                    //#line 490 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 490 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 490 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 490 GJavaParser.g
            );
            break;
        }
                //
                // Rule 179:  EnumConstant ::= Annotationsopt identifier Argumentsopt ClassBodyopt
                //
        case 179: {
            //#line 492 "GJavaParser.g"
            setResult(
                //#line 492 GJavaParser.g
                ast_pool.data.Next() = new EnumConstant(getLeftIToken(), getRightIToken(),
                    //#line 492 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 492 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 492 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 492 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 492 GJavaParser.g
            );
            break;
        }
                //
                // Rule 180:  Arguments ::= ( ArgumentListopt )
                //
        case 180: {
            //#line 494 "GJavaParser.g"
            setResult(
                //#line 494 GJavaParser.g
                ast_pool.data.Next() = new Arguments(getLeftIToken(), getRightIToken(),
                    //#line 494 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 494 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 494 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 494 GJavaParser.g
            );
            break;
        }
                //
                // Rule 181:  EnumBodyDeclarations ::= ; ClassBodyDeclarationsopt
                //
        case 181: {
            //#line 496 "GJavaParser.g"
            setResult(
                //#line 496 GJavaParser.g
                ast_pool.data.Next() = new EnumBodyDeclarations(getLeftIToken(), getRightIToken(),
                    //#line 496 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 496 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 496 GJavaParser.g
            );
            break;
        }
                //
                // Rule 182:  InterfaceDeclaration ::= NormalInterfaceDeclaration
                //
        case 182:
            break;
            //
            // Rule 183:  InterfaceDeclaration ::= AnnotationTypeDeclaration
            //
        case 183:
            break;
            //
            // Rule 184:  NormalInterfaceDeclaration ::= InterfaceModifiersopt interface identifier TypeParametersopt ExtendsInterfacesopt InterfaceBody
            //
        case 184: {
            //#line 503 "GJavaParser.g"
            setResult(
                //#line 503 GJavaParser.g
                ast_pool.data.Next() = new NormalInterfaceDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 503 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 503 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 503 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 503 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 503 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 503 GJavaParser.g
                    (IAst*)getRhsSym(6))
                //#line 503 GJavaParser.g
            );
            break;
        }
                //
                // Rule 185:  InterfaceModifiers ::= InterfaceModifier
                //
        case 185:
            break;
            //
            // Rule 186:  InterfaceModifiers ::= InterfaceModifiers InterfaceModifier
            //
        case 186: {
            //#line 506 "GJavaParser.g"
            setResult(
                //#line 506 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifiers(getLeftIToken(), getRightIToken(),
                    //#line 506 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 506 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 506 GJavaParser.g
            );
            break;
        }
                //
                // Rule 187:  InterfaceModifier ::= Annotation
                //
        case 187:
            break;
            //
            // Rule 188:  InterfaceModifier ::= public
            //
        case 188: {
            //#line 509 "GJavaParser.g"
            setResult(
                //#line 509 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifier0(getRhsIToken(1))
                //#line 509 GJavaParser.g
            );
            break;
        }
                //
                // Rule 189:  InterfaceModifier ::= protected
                //
        case 189: {
            //#line 510 "GJavaParser.g"
            setResult(
                //#line 510 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifier1(getRhsIToken(1))
                //#line 510 GJavaParser.g
            );
            break;
        }
                //
                // Rule 190:  InterfaceModifier ::= private
                //
        case 190: {
            //#line 511 "GJavaParser.g"
            setResult(
                //#line 511 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifier2(getRhsIToken(1))
                //#line 511 GJavaParser.g
            );
            break;
        }
                //
                // Rule 191:  InterfaceModifier ::= abstract
                //
        case 191: {
            //#line 512 "GJavaParser.g"
            setResult(
                //#line 512 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifier3(getRhsIToken(1))
                //#line 512 GJavaParser.g
            );
            break;
        }
                //
                // Rule 192:  InterfaceModifier ::= static
                //
        case 192: {
            //#line 513 "GJavaParser.g"
            setResult(
                //#line 513 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifier4(getRhsIToken(1))
                //#line 513 GJavaParser.g
            );
            break;
        }
                //
                // Rule 193:  InterfaceModifier ::= strictfp
                //
        case 193: {
            //#line 514 "GJavaParser.g"
            setResult(
                //#line 514 GJavaParser.g
                ast_pool.data.Next() = new InterfaceModifier5(getRhsIToken(1))
                //#line 514 GJavaParser.g
            );
            break;
        }
                //
                // Rule 194:  ExtendsInterfaces ::= extends InterfaceType
                //
        case 194: {
            //#line 516 "GJavaParser.g"
            setResult(
                //#line 516 GJavaParser.g
                ast_pool.data.Next() = new ExtendsInterfaces0(getLeftIToken(), getRightIToken(),
                    //#line 516 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 516 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 516 GJavaParser.g
            );
            break;
        }
                //
                // Rule 195:  ExtendsInterfaces ::= ExtendsInterfaces , InterfaceType
                //
        case 195: {
            //#line 517 "GJavaParser.g"
            setResult(
                //#line 517 GJavaParser.g
                ast_pool.data.Next() = new ExtendsInterfaces1(getLeftIToken(), getRightIToken(),
                    //#line 517 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 517 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 517 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 517 GJavaParser.g
            );
            break;
        }
                //
                // Rule 196:  InterfaceBody ::= { InterfaceMemberDeclarationsopt }
                //
        case 196: {
            //#line 524 "GJavaParser.g"
            setResult(
                //#line 524 GJavaParser.g
                ast_pool.data.Next() = new InterfaceBody(getLeftIToken(), getRightIToken(),
                    //#line 524 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 524 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 524 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 524 GJavaParser.g
            );
            break;
        }
                //
                // Rule 197:  InterfaceMemberDeclarations ::= InterfaceMemberDeclaration
                //
        case 197:
            break;
            //
            // Rule 198:  InterfaceMemberDeclarations ::= InterfaceMemberDeclarations InterfaceMemberDeclaration
            //
        case 198: {
            //#line 527 "GJavaParser.g"
            setResult(
                //#line 527 GJavaParser.g
                ast_pool.data.Next() = new InterfaceMemberDeclarations(getLeftIToken(), getRightIToken(),
                    //#line 527 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 527 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 527 GJavaParser.g
            );
            break;
        }
                //
                // Rule 199:  InterfaceMemberDeclaration ::= ConstantDeclaration
                //
        case 199:
            break;
            //
            // Rule 200:  InterfaceMemberDeclaration ::= AbstractMethodDeclaration
            //
        case 200:
            break;
            //
            // Rule 201:  InterfaceMemberDeclaration ::= ClassDeclaration
            //
        case 201:
            break;
            //
            // Rule 202:  InterfaceMemberDeclaration ::= InterfaceDeclaration
            //
        case 202:
            break;
            //
            // Rule 203:  InterfaceMemberDeclaration ::= ;
            //
        case 203: {
            //#line 533 "GJavaParser.g"
            setResult(
                //#line 533 GJavaParser.g
                ast_pool.data.Next() = new InterfaceMemberDeclaration(getRhsIToken(1))
                //#line 533 GJavaParser.g
            );
            break;
        }
                //
                // Rule 204:  ConstantDeclaration ::= ConstantModifiersopt Type VariableDeclarators
                //
        case 204: {
            //#line 535 "GJavaParser.g"
            setResult(
                //#line 535 GJavaParser.g
                ast_pool.data.Next() = new ConstantDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 535 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 535 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 535 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 535 GJavaParser.g
            );
            break;
        }
                //
                // Rule 205:  ConstantModifiers ::= ConstantModifier
                //
        case 205:
            break;
            //
            // Rule 206:  ConstantModifiers ::= ConstantModifiers ConstantModifier
            //
        case 206: {
            //#line 538 "GJavaParser.g"
            setResult(
                //#line 538 GJavaParser.g
                ast_pool.data.Next() = new ConstantModifiers(getLeftIToken(), getRightIToken(),
                    //#line 538 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 538 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 538 GJavaParser.g
            );
            break;
        }
                //
                // Rule 207:  ConstantModifier ::= Annotation
                //
        case 207:
            break;
            //
            // Rule 208:  ConstantModifier ::= public
            //
        case 208: {
            //#line 541 "GJavaParser.g"
            setResult(
                //#line 541 GJavaParser.g
                ast_pool.data.Next() = new ConstantModifier0(getRhsIToken(1))
                //#line 541 GJavaParser.g
            );
            break;
        }
                //
                // Rule 209:  ConstantModifier ::= static
                //
        case 209: {
            //#line 542 "GJavaParser.g"
            setResult(
                //#line 542 GJavaParser.g
                ast_pool.data.Next() = new ConstantModifier1(getRhsIToken(1))
                //#line 542 GJavaParser.g
            );
            break;
        }
                //
                // Rule 210:  ConstantModifier ::= final
                //
        case 210: {
            //#line 543 "GJavaParser.g"
            setResult(
                //#line 543 GJavaParser.g
                ast_pool.data.Next() = new ConstantModifier2(getRhsIToken(1))
                //#line 543 GJavaParser.g
            );
            break;
        }
                //
                // Rule 211:  AbstractMethodDeclaration ::= AbstractMethodModifiersopt TypeParametersopt ResultType MethodDeclarator Throwsopt ;
                //
        case 211: {
            //#line 545 "GJavaParser.g"
            setResult(
                //#line 545 GJavaParser.g
                ast_pool.data.Next() = new AbstractMethodDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 545 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 545 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 545 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 545 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 545 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 545 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)))
                //#line 545 GJavaParser.g
            );
            break;
        }
                //
                // Rule 212:  AbstractMethodModifiers ::= AbstractMethodModifier
                //
        case 212:
            break;
            //
            // Rule 213:  AbstractMethodModifiers ::= AbstractMethodModifiers AbstractMethodModifier
            //
        case 213: {
            //#line 548 "GJavaParser.g"
            setResult(
                //#line 548 GJavaParser.g
                ast_pool.data.Next() = new AbstractMethodModifiers(getLeftIToken(), getRightIToken(),
                    //#line 548 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 548 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 548 GJavaParser.g
            );
            break;
        }
                //
                // Rule 214:  AbstractMethodModifier ::= Annotations
                //
        case 214:
            break;
            //
            // Rule 215:  AbstractMethodModifier ::= public
            //
        case 215: {
            //#line 551 "GJavaParser.g"
            setResult(
                //#line 551 GJavaParser.g
                ast_pool.data.Next() = new AbstractMethodModifier0(getRhsIToken(1))
                //#line 551 GJavaParser.g
            );
            break;
        }
                //
                // Rule 216:  AbstractMethodModifier ::= abstract
                //
        case 216: {
            //#line 552 "GJavaParser.g"
            setResult(
                //#line 552 GJavaParser.g
                ast_pool.data.Next() = new AbstractMethodModifier1(getRhsIToken(1))
                //#line 552 GJavaParser.g
            );
            break;
        }
                //
                // Rule 217:  AnnotationTypeDeclaration ::= InterfaceModifiersopt @ interface identifier AnnotationTypeBody
                //
        case 217: {
            //#line 554 "GJavaParser.g"
            setResult(
                //#line 554 GJavaParser.g
                ast_pool.data.Next() = new AnnotationTypeDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 554 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 554 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 554 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 554 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 554 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 554 GJavaParser.g
            );
            break;
        }
                //
                // Rule 218:  AnnotationTypeBody ::= { AnnotationTypeElementDeclarationsopt }
                //
        case 218: {
            //#line 556 "GJavaParser.g"
            setResult(
                //#line 556 GJavaParser.g
                ast_pool.data.Next() = new AnnotationTypeBody(getLeftIToken(), getRightIToken(),
                    //#line 556 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 556 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 556 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 556 GJavaParser.g
            );
            break;
        }
                //
                // Rule 219:  AnnotationTypeElementDeclarations ::= AnnotationTypeElementDeclaration
                //
        case 219:
            break;
            //
            // Rule 220:  AnnotationTypeElementDeclarations ::= AnnotationTypeElementDeclarations AnnotationTypeElementDeclaration
            //
        case 220: {
            //#line 559 "GJavaParser.g"
            setResult(
                //#line 559 GJavaParser.g
                ast_pool.data.Next() = new AnnotationTypeElementDeclarations(getLeftIToken(), getRightIToken(),
                    //#line 559 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 559 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 559 GJavaParser.g
            );
            break;
        }
                //
                // Rule 221:  AnnotationTypeElementDeclaration ::= AbstractMethodModifiersopt Type identifier ( ) DefaultValueopt ;
                //
        case 221: {
            //#line 561 "GJavaParser.g"
            setResult(
                //#line 561 GJavaParser.g
                ast_pool.data.Next() = new AnnotationTypeElementDeclaration0(getLeftIToken(), getRightIToken(),
                    //#line 561 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 561 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 561 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 561 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 561 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 561 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 561 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)))
                //#line 561 GJavaParser.g
            );
            break;
        }
                //
                // Rule 222:  AnnotationTypeElementDeclaration ::= ConstantDeclaration
                //
        case 222:
            break;
            //
            // Rule 223:  AnnotationTypeElementDeclaration ::= ClassDeclaration
            //
        case 223:
            break;
            //
            // Rule 224:  AnnotationTypeElementDeclaration ::= InterfaceDeclaration
            //
        case 224:
            break;
            //
            // Rule 225:  AnnotationTypeElementDeclaration ::= EnumDeclaration
            //
        case 225:
            break;
            //
            // Rule 226:  AnnotationTypeElementDeclaration ::= AnnotationTypeDeclaration
            //
        case 226:
            break;
            //
            // Rule 227:  AnnotationTypeElementDeclaration ::= ;
            //
        case 227: {
            //#line 567 "GJavaParser.g"
            setResult(
                //#line 567 GJavaParser.g
                ast_pool.data.Next() = new AnnotationTypeElementDeclaration1(getRhsIToken(1))
                //#line 567 GJavaParser.g
            );
            break;
        }
                //
                // Rule 228:  DefaultValue ::= default ElementValue
                //
        case 228: {
            //#line 569 "GJavaParser.g"
            setResult(
                //#line 569 GJavaParser.g
                ast_pool.data.Next() = new DefaultValue(getLeftIToken(), getRightIToken(),
                    //#line 569 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 569 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 569 GJavaParser.g
            );
            break;
        }
                //
                // Rule 229:  Annotations ::= Annotation
                //
        case 229:
            break;
            //
            // Rule 230:  Annotations ::= Annotations Annotation
            //
        case 230: {
            //#line 572 "GJavaParser.g"
            setResult(
                //#line 572 GJavaParser.g
                ast_pool.data.Next() = new Annotations(getLeftIToken(), getRightIToken(),
                    //#line 572 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 572 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 572 GJavaParser.g
            );
            break;
        }
                //
                // Rule 231:  Annotation ::= NormalAnnotation
                //
        case 231:
            break;
            //
            // Rule 232:  Annotation ::= MarkerAnnotation
            //
        case 232:
            break;
            //
            // Rule 233:  Annotation ::= SingleElementAnnotation
            //
        case 233:
            break;
            //
            // Rule 234:  NormalAnnotation ::= @ TypeName ( ElementValuePairsopt )
            //
        case 234: {
            //#line 578 "GJavaParser.g"
            setResult(
                //#line 578 GJavaParser.g
                ast_pool.data.Next() = new NormalAnnotation(getLeftIToken(), getRightIToken(),
                    //#line 578 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 578 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 578 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 578 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 578 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)))
                //#line 578 GJavaParser.g
            );
            break;
        }
                //
                // Rule 235:  ElementValuePairs ::= ElementValuePair
                //
        case 235:
            break;
            //
            // Rule 236:  ElementValuePairs ::= ElementValuePairs , ElementValuePair
            //
        case 236: {
            //#line 581 "GJavaParser.g"
            setResult(
                //#line 581 GJavaParser.g
                ast_pool.data.Next() = new ElementValuePairs(getLeftIToken(), getRightIToken(),
                    //#line 581 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 581 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 581 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 581 GJavaParser.g
            );
            break;
        }
                //
                // Rule 237:  ElementValuePair ::= SimpleName = ElementValue
                //
        case 237: {
            //#line 583 "GJavaParser.g"
            setResult(
                //#line 583 GJavaParser.g
                ast_pool.data.Next() = new ElementValuePair(getLeftIToken(), getRightIToken(),
                    //#line 583 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 583 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 583 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 583 GJavaParser.g
            );
            break;
        }
                //
                // Rule 238:  SimpleName ::= identifier
                //
        case 238:
            break;
            //
            // Rule 239:  ElementValue ::= ConditionalExpression
            //
        case 239:
            break;
            //
            // Rule 240:  ElementValue ::= Annotation
            //
        case 240:
            break;
            //
            // Rule 241:  ElementValue ::= ElementValueArrayInitializer
            //
        case 241:
            break;
            //
            // Rule 242:  ElementValueArrayInitializer ::= { ElementValuesopt ,opt }
            //
        case 242: {
            //#line 591 "GJavaParser.g"
            setResult(
                //#line 591 GJavaParser.g
                ast_pool.data.Next() = new ElementValueArrayInitializer(getLeftIToken(), getRightIToken(),
                    //#line 591 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 591 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 591 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 591 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 591 GJavaParser.g
            );
            break;
        }
                //
                // Rule 243:  ElementValues ::= ElementValue
                //
        case 243:
            break;
            //
            // Rule 244:  ElementValues ::= ElementValues , ElementValue
            //
        case 244: {
            //#line 594 "GJavaParser.g"
            setResult(
                //#line 594 GJavaParser.g
                ast_pool.data.Next() = new ElementValues(getLeftIToken(), getRightIToken(),
                    //#line 594 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 594 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 594 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 594 GJavaParser.g
            );
            break;
        }
                //
                // Rule 245:  MarkerAnnotation ::= @ TypeName
                //
        case 245: {
            //#line 596 "GJavaParser.g"
            setResult(
                //#line 596 GJavaParser.g
                ast_pool.data.Next() = new MarkerAnnotation(getLeftIToken(), getRightIToken(),
                    //#line 596 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 596 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 596 GJavaParser.g
            );
            break;
        }
                //
                // Rule 246:  SingleElementAnnotation ::= @ TypeName ( ElementValue )
                //
        case 246: {
            //#line 598 "GJavaParser.g"
            setResult(
                //#line 598 GJavaParser.g
                ast_pool.data.Next() = new SingleElementAnnotation(getLeftIToken(), getRightIToken(),
                    //#line 598 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 598 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 598 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 598 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 598 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)))
                //#line 598 GJavaParser.g
            );
            break;
        }
                //
                // Rule 247:  ArrayInitializer ::= { VariableInitializersopt ,opt }
                //
        case 247: {
            //#line 602 "GJavaParser.g"
            setResult(
                //#line 602 GJavaParser.g
                ast_pool.data.Next() = new ArrayInitializer(getLeftIToken(), getRightIToken(),
                    //#line 602 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 602 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 602 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 602 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 602 GJavaParser.g
            );
            break;
        }
                //
                // Rule 248:  VariableInitializers ::= VariableInitializer
                //
        case 248:
            break;
            //
            // Rule 249:  VariableInitializers ::= VariableInitializers , VariableInitializer
            //
        case 249: {
            //#line 605 "GJavaParser.g"
            setResult(
                //#line 605 GJavaParser.g
                ast_pool.data.Next() = new VariableInitializers(getLeftIToken(), getRightIToken(),
                    //#line 605 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 605 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 605 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 605 GJavaParser.g
            );
            break;
        }
                //
                // Rule 250:  Block ::= { BlockStatementsopt }
                //
        case 250: {
            //#line 621 "GJavaParser.g"
            setResult(
                //#line 621 GJavaParser.g
                ast_pool.data.Next() = new Block(getLeftIToken(), getRightIToken(),
                    //#line 621 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 621 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 621 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 621 GJavaParser.g
            );
            break;
        }
                //
                // Rule 251:  BlockStatements ::= BlockStatement
                //
        case 251:
            break;
            //
            // Rule 252:  BlockStatements ::= BlockStatements BlockStatement
            //
        case 252: {
            //#line 624 "GJavaParser.g"
            setResult(
                //#line 624 GJavaParser.g
                ast_pool.data.Next() = new BlockStatements(getLeftIToken(), getRightIToken(),
                    //#line 624 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 624 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 624 GJavaParser.g
            );
            break;
        }
                //
                // Rule 253:  BlockStatement ::= LocalVariableDeclarationStatement
                //
        case 253:
            break;
            //
            // Rule 254:  BlockStatement ::= ClassDeclaration
            //
        case 254:
            break;
            //
            // Rule 255:  BlockStatement ::= Statement
            //
        case 255:
            break;
            //
            // Rule 256:  LocalVariableDeclarationStatement ::= LocalVariableDeclaration ;
            //
        case 256: {
            //#line 630 "GJavaParser.g"
            setResult(
                //#line 630 GJavaParser.g
                ast_pool.data.Next() = new LocalVariableDeclarationStatement(getLeftIToken(), getRightIToken(),
                    //#line 630 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 630 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)))
                //#line 630 GJavaParser.g
            );
            break;
        }
                //
                // Rule 257:  LocalVariableDeclaration ::= VariableModifiersopt Type VariableDeclarators
                //
        case 257: {
            //#line 632 "GJavaParser.g"
            setResult(
                //#line 632 GJavaParser.g
                ast_pool.data.Next() = new LocalVariableDeclaration(getLeftIToken(), getRightIToken(),
                    //#line 632 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 632 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 632 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 632 GJavaParser.g
            );
            break;
        }
                //
                // Rule 258:  Statement ::= StatementWithoutTrailingSubstatement
                //
        case 258:
            break;
            //
            // Rule 259:  Statement ::= LabeledStatement
            //
        case 259:
            break;
            //
            // Rule 260:  Statement ::= IfThenStatement
            //
        case 260:
            break;
            //
            // Rule 261:  Statement ::= IfThenElseStatement
            //
        case 261:
            break;
            //
            // Rule 262:  Statement ::= WhileStatement
            //
        case 262:
            break;
            //
            // Rule 263:  Statement ::= ForStatement
            //
        case 263:
            break;
            //
            // Rule 264:  StatementWithoutTrailingSubstatement ::= Block
            //
        case 264:
            break;
            //
            // Rule 265:  StatementWithoutTrailingSubstatement ::= EmptyStatement
            //
        case 265:
            break;
            //
            // Rule 266:  StatementWithoutTrailingSubstatement ::= ExpressionStatement
            //
        case 266:
            break;
            //
            // Rule 267:  StatementWithoutTrailingSubstatement ::= AssertStatement
            //
        case 267:
            break;
            //
            // Rule 268:  StatementWithoutTrailingSubstatement ::= SwitchStatement
            //
        case 268:
            break;
            //
            // Rule 269:  StatementWithoutTrailingSubstatement ::= DoStatement
            //
        case 269:
            break;
            //
            // Rule 270:  StatementWithoutTrailingSubstatement ::= BreakStatement
            //
        case 270:
            break;
            //
            // Rule 271:  StatementWithoutTrailingSubstatement ::= ContinueStatement
            //
        case 271:
            break;
            //
            // Rule 272:  StatementWithoutTrailingSubstatement ::= ReturnStatement
            //
        case 272:
            break;
            //
            // Rule 273:  StatementWithoutTrailingSubstatement ::= SynchronizedStatement
            //
        case 273:
            break;
            //
            // Rule 274:  StatementWithoutTrailingSubstatement ::= ThrowStatement
            //
        case 274:
            break;
            //
            // Rule 275:  StatementWithoutTrailingSubstatement ::= TryStatement
            //
        case 275:
            break;
            //
            // Rule 276:  StatementNoShortIf ::= StatementWithoutTrailingSubstatement
            //
        case 276:
            break;
            //
            // Rule 277:  StatementNoShortIf ::= LabeledStatementNoShortIf
            //
        case 277:
            break;
            //
            // Rule 278:  StatementNoShortIf ::= IfThenElseStatementNoShortIf
            //
        case 278:
            break;
            //
            // Rule 279:  StatementNoShortIf ::= WhileStatementNoShortIf
            //
        case 279:
            break;
            //
            // Rule 280:  StatementNoShortIf ::= ForStatementNoShortIf
            //
        case 280:
            break;
            //
            // Rule 281:  IfThenStatement ::= if ( Expression ) Statement
            //
        case 281: {
            //#line 675 "GJavaParser.g"
            setResult(
                //#line 675 GJavaParser.g
                ast_pool.data.Next() = new IfThenStatement(getLeftIToken(), getRightIToken(),
                    //#line 675 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 675 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 675 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 675 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 675 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 675 GJavaParser.g
            );
            break;
        }
                //
                // Rule 282:  IfThenElseStatement ::= if ( Expression ) StatementNoShortIf else Statement
                //
        case 282: {
            //#line 677 "GJavaParser.g"
            setResult(
                //#line 677 GJavaParser.g
                ast_pool.data.Next() = new IfThenElseStatement(getLeftIToken(), getRightIToken(),
                    //#line 677 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 677 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 677 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 677 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 677 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 677 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)),
                    //#line 677 GJavaParser.g
                    (IAst*)getRhsSym(7))
                //#line 677 GJavaParser.g
            );
            break;
        }
                //
                // Rule 283:  IfThenElseStatementNoShortIf ::= if ( Expression ) StatementNoShortIf else StatementNoShortIf
                //
        case 283: {
            //#line 679 "GJavaParser.g"
            setResult(
                //#line 679 GJavaParser.g
                ast_pool.data.Next() = new IfThenElseStatementNoShortIf(getLeftIToken(), getRightIToken(),
                    //#line 679 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 679 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 679 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 679 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 679 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 679 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)),
                    //#line 679 GJavaParser.g
                    (IAst*)getRhsSym(7))
                //#line 679 GJavaParser.g
            );
            break;
        }
                //
                // Rule 284:  EmptyStatement ::= ;
                //
        case 284: {
            //#line 681 "GJavaParser.g"
            setResult(
                //#line 681 GJavaParser.g
                ast_pool.data.Next() = new EmptyStatement(getRhsIToken(1))
                //#line 681 GJavaParser.g
            );
            break;
        }
                //
                // Rule 285:  LabeledStatement ::= identifier : Statement
                //
        case 285: {
            //#line 683 "GJavaParser.g"
            setResult(
                //#line 683 GJavaParser.g
                ast_pool.data.Next() = new LabeledStatement(getLeftIToken(), getRightIToken(),
                    //#line 683 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 683 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 683 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 683 GJavaParser.g
            );
            break;
        }
                //
                // Rule 286:  LabeledStatementNoShortIf ::= identifier : StatementNoShortIf
                //
        case 286: {
            //#line 685 "GJavaParser.g"
            setResult(
                //#line 685 GJavaParser.g
                ast_pool.data.Next() = new LabeledStatementNoShortIf(getLeftIToken(), getRightIToken(),
                    //#line 685 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 685 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 685 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 685 GJavaParser.g
            );
            break;
        }
                //
                // Rule 287:  ExpressionStatement ::= StatementExpression ;
                //
        case 287: {
            //#line 687 "GJavaParser.g"
            setResult(
                //#line 687 GJavaParser.g
                ast_pool.data.Next() = new ExpressionStatement(getLeftIToken(), getRightIToken(),
                    //#line 687 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 687 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)))
                //#line 687 GJavaParser.g
            );
            break;
        }
                //
                // Rule 288:  StatementExpression ::= Assignment
                //
        case 288:
            break;
            //
            // Rule 289:  StatementExpression ::= PreIncrementExpression
            //
        case 289:
            break;
            //
            // Rule 290:  StatementExpression ::= PreDecrementExpression
            //
        case 290:
            break;
            //
            // Rule 291:  StatementExpression ::= PostIncrementExpression
            //
        case 291:
            break;
            //
            // Rule 292:  StatementExpression ::= PostDecrementExpression
            //
        case 292:
            break;
            //
            // Rule 293:  StatementExpression ::= MethodInvocation
            //
        case 293:
            break;
            //
            // Rule 294:  StatementExpression ::= ClassInstanceCreationExpression
            //
        case 294:
            break;
            //
            // Rule 295:  AssertStatement ::= assert Expression ;
            //
        case 295: {
            //#line 706 "GJavaParser.g"
            setResult(
                //#line 706 GJavaParser.g
                ast_pool.data.Next() = new AssertStatement0(getLeftIToken(), getRightIToken(),
                    //#line 706 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 706 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 706 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 706 GJavaParser.g
            );
            break;
        }
                //
                // Rule 296:  AssertStatement ::= assert Expression : Expression ;
                //
        case 296: {
            //#line 707 "GJavaParser.g"
            setResult(
                //#line 707 GJavaParser.g
                ast_pool.data.Next() = new AssertStatement1(getLeftIToken(), getRightIToken(),
                    //#line 707 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 707 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 707 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 707 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 707 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)))
                //#line 707 GJavaParser.g
            );
            break;
        }
                //
                // Rule 297:  SwitchStatement ::= switch ( Expression ) SwitchBlock
                //
        case 297: {
            //#line 709 "GJavaParser.g"
            setResult(
                //#line 709 GJavaParser.g
                ast_pool.data.Next() = new SwitchStatement(getLeftIToken(), getRightIToken(),
                    //#line 709 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 709 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 709 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 709 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 709 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 709 GJavaParser.g
            );
            break;
        }
                //
                // Rule 298:  SwitchBlock ::= { SwitchBlockStatementGroupsopt SwitchLabelsopt }
                //
        case 298: {
            //#line 711 "GJavaParser.g"
            setResult(
                //#line 711 GJavaParser.g
                ast_pool.data.Next() = new SwitchBlock(getLeftIToken(), getRightIToken(),
                    //#line 711 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 711 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 711 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 711 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 711 GJavaParser.g
            );
            break;
        }
                //
                // Rule 299:  SwitchBlockStatementGroups ::= SwitchBlockStatementGroup
                //
        case 299:
            break;
            //
            // Rule 300:  SwitchBlockStatementGroups ::= SwitchBlockStatementGroups SwitchBlockStatementGroup
            //
        case 300: {
            //#line 714 "GJavaParser.g"
            setResult(
                //#line 714 GJavaParser.g
                ast_pool.data.Next() = new SwitchBlockStatementGroups(getLeftIToken(), getRightIToken(),
                    //#line 714 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 714 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 714 GJavaParser.g
            );
            break;
        }
                //
                // Rule 301:  SwitchBlockStatementGroup ::= SwitchLabels BlockStatements
                //
        case 301: {
            //#line 716 "GJavaParser.g"
            setResult(
                //#line 716 GJavaParser.g
                ast_pool.data.Next() = new SwitchBlockStatementGroup(getLeftIToken(), getRightIToken(),
                    //#line 716 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 716 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 716 GJavaParser.g
            );
            break;
        }
                //
                // Rule 302:  SwitchLabels ::= SwitchLabel
                //
        case 302:
            break;
            //
            // Rule 303:  SwitchLabels ::= SwitchLabels SwitchLabel
            //
        case 303: {
            //#line 719 "GJavaParser.g"
            setResult(
                //#line 719 GJavaParser.g
                ast_pool.data.Next() = new SwitchLabels(getLeftIToken(), getRightIToken(),
                    //#line 719 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 719 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 719 GJavaParser.g
            );
            break;
        }
                //
                // Rule 304:  SwitchLabel ::= case ConstantExpression :
                //
        case 304: {
            //#line 721 "GJavaParser.g"
            setResult(
                //#line 721 GJavaParser.g
                ast_pool.data.Next() = new SwitchLabel0(getLeftIToken(), getRightIToken(),
                    //#line 721 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 721 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 721 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 721 GJavaParser.g
            );
            break;
        }
                //
                // Rule 305:  SwitchLabel ::= case EnumConstant :
                //
        case 305: {
            //#line 722 "GJavaParser.g"
            setResult(
                //#line 722 GJavaParser.g
                ast_pool.data.Next() = new SwitchLabel1(getLeftIToken(), getRightIToken(),
                    //#line 722 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 722 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 722 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 722 GJavaParser.g
            );
            break;
        }
                //
                // Rule 306:  SwitchLabel ::= default :
                //
        case 306: {
            //#line 723 "GJavaParser.g"
            setResult(
                //#line 723 GJavaParser.g
                ast_pool.data.Next() = new SwitchLabel2(getLeftIToken(), getRightIToken(),
                    //#line 723 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 723 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)))
                //#line 723 GJavaParser.g
            );
            break;
        }
                //
                // Rule 307:  EnumConstant ::= identifier
                //
        case 307:
            break;
            //
            // Rule 308:  WhileStatement ::= while ( Expression ) Statement
            //
        case 308: {
            //#line 727 "GJavaParser.g"
            setResult(
                //#line 727 GJavaParser.g
                ast_pool.data.Next() = new WhileStatement(getLeftIToken(), getRightIToken(),
                    //#line 727 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 727 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 727 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 727 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 727 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 727 GJavaParser.g
            );
            break;
        }
                //
                // Rule 309:  WhileStatementNoShortIf ::= while ( Expression ) StatementNoShortIf
                //
        case 309: {
            //#line 729 "GJavaParser.g"
            setResult(
                //#line 729 GJavaParser.g
                ast_pool.data.Next() = new WhileStatementNoShortIf(getLeftIToken(), getRightIToken(),
                    //#line 729 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 729 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 729 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 729 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 729 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 729 GJavaParser.g
            );
            break;
        }
                //
                // Rule 310:  DoStatement ::= do Statement while ( Expression ) ;
                //
        case 310: {
            //#line 731 "GJavaParser.g"
            setResult(
                //#line 731 GJavaParser.g
                ast_pool.data.Next() = new DoStatement(getLeftIToken(), getRightIToken(),
                    //#line 731 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 731 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 731 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 731 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 731 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 731 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)),
                    //#line 731 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)))
                //#line 731 GJavaParser.g
            );
            break;
        }
                //
                // Rule 311:  ForStatement ::= BasicForStatement
                //
        case 311:
            break;
            //
            // Rule 312:  ForStatement ::= EnhancedForStatement
            //
        case 312:
            break;
            //
            // Rule 313:  BasicForStatement ::= for ( ForInitopt ; Expressionopt ; ForUpdateopt ) Statement
            //
        case 313: {
            //#line 736 "GJavaParser.g"
            setResult(
                //#line 736 GJavaParser.g
                ast_pool.data.Next() = new BasicForStatement(getLeftIToken(), getRightIToken(),
                    //#line 736 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 736 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 736 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 736 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 736 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 736 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)),
                    //#line 736 GJavaParser.g
                    (IAst*)getRhsSym(7),
                    //#line 736 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(8)),
                    //#line 736 GJavaParser.g
                    (IAst*)getRhsSym(9))
                //#line 736 GJavaParser.g
            );
            break;
        }
                //
                // Rule 314:  ForStatementNoShortIf ::= for ( ForInitopt ; Expressionopt ; ForUpdateopt ) StatementNoShortIf
                //
        case 314: {
            //#line 738 "GJavaParser.g"
            setResult(
                //#line 738 GJavaParser.g
                ast_pool.data.Next() = new ForStatementNoShortIf(getLeftIToken(), getRightIToken(),
                    //#line 738 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 738 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 738 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 738 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 738 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 738 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)),
                    //#line 738 GJavaParser.g
                    (IAst*)getRhsSym(7),
                    //#line 738 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(8)),
                    //#line 738 GJavaParser.g
                    (IAst*)getRhsSym(9))
                //#line 738 GJavaParser.g
            );
            break;
        }
                //
                // Rule 315:  ForInit ::= StatementExpressionList
                //
        case 315:
            break;
            //
            // Rule 316:  ForInit ::= LocalVariableDeclaration
            //
        case 316:
            break;
            //
            // Rule 317:  ForUpdate ::= StatementExpressionList
            //
        case 317:
            break;
            //
            // Rule 318:  StatementExpressionList ::= StatementExpression
            //
        case 318:
            break;
            //
            // Rule 319:  StatementExpressionList ::= StatementExpressionList , StatementExpression
            //
        case 319: {
            //#line 746 "GJavaParser.g"
            setResult(
                //#line 746 GJavaParser.g
                ast_pool.data.Next() = new StatementExpressionList(getLeftIToken(), getRightIToken(),
                    //#line 746 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 746 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 746 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 746 GJavaParser.g
            );
            break;
        }
                //
                // Rule 320:  EnhancedForStatement ::= for ( FormalParameter : Expression ) Statement
                //
        case 320: {
            //#line 748 "GJavaParser.g"
            setResult(
                //#line 748 GJavaParser.g
                ast_pool.data.Next() = new EnhancedForStatement(getLeftIToken(), getRightIToken(),
                    //#line 748 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 748 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 748 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 748 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 748 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 748 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(6)),
                    //#line 748 GJavaParser.g
                    (IAst*)getRhsSym(7))
                //#line 748 GJavaParser.g
            );
            break;
        }
                //
                // Rule 321:  BreakStatement ::= break identifieropt ;
                //
        case 321: {
            //#line 750 "GJavaParser.g"
            setResult(
                //#line 750 GJavaParser.g
                ast_pool.data.Next() = new BreakStatement(getLeftIToken(), getRightIToken(),
                    //#line 750 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 750 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 750 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 750 GJavaParser.g
            );
            break;
        }
                //
                // Rule 322:  ContinueStatement ::= continue identifieropt ;
                //
        case 322: {
            //#line 752 "GJavaParser.g"
            setResult(
                //#line 752 GJavaParser.g
                ast_pool.data.Next() = new ContinueStatement(getLeftIToken(), getRightIToken(),
                    //#line 752 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 752 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 752 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 752 GJavaParser.g
            );
            break;
        }
                //
                // Rule 323:  ReturnStatement ::= return Expressionopt ;
                //
        case 323: {
            //#line 754 "GJavaParser.g"
            setResult(
                //#line 754 GJavaParser.g
                ast_pool.data.Next() = new ReturnStatement(getLeftIToken(), getRightIToken(),
                    //#line 754 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 754 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 754 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 754 GJavaParser.g
            );
            break;
        }
                //
                // Rule 324:  ThrowStatement ::= throw Expression ;
                //
        case 324: {
            //#line 756 "GJavaParser.g"
            setResult(
                //#line 756 GJavaParser.g
                ast_pool.data.Next() = new ThrowStatement(getLeftIToken(), getRightIToken(),
                    //#line 756 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 756 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 756 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 756 GJavaParser.g
            );
            break;
        }
                //
                // Rule 325:  SynchronizedStatement ::= synchronized ( Expression ) Block
                //
        case 325: {
            //#line 758 "GJavaParser.g"
            setResult(
                //#line 758 GJavaParser.g
                ast_pool.data.Next() = new SynchronizedStatement(getLeftIToken(), getRightIToken(),
                    //#line 758 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 758 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 758 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 758 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 758 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 758 GJavaParser.g
            );
            break;
        }
                //
                // Rule 326:  TryStatement ::= try Block Catches
                //
        case 326: {
            //#line 760 "GJavaParser.g"
            setResult(
                //#line 760 GJavaParser.g
                ast_pool.data.Next() = new TryStatement0(getLeftIToken(), getRightIToken(),
                    //#line 760 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 760 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 760 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 760 GJavaParser.g
            );
            break;
        }
                //
                // Rule 327:  TryStatement ::= try Block Catchesopt Finally
                //
        case 327: {
            //#line 761 "GJavaParser.g"
            setResult(
                //#line 761 GJavaParser.g
                ast_pool.data.Next() = new TryStatement1(getLeftIToken(), getRightIToken(),
                    //#line 761 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 761 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 761 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 761 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 761 GJavaParser.g
            );
            break;
        }
                //
                // Rule 328:  Catches ::= CatchClause
                //
        case 328:
            break;
            //
            // Rule 329:  Catches ::= Catches CatchClause
            //
        case 329: {
            //#line 764 "GJavaParser.g"
            setResult(
                //#line 764 GJavaParser.g
                ast_pool.data.Next() = new Catches(getLeftIToken(), getRightIToken(),
                    //#line 764 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 764 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 764 GJavaParser.g
            );
            break;
        }
                //
                // Rule 330:  CatchClause ::= catch ( FormalParameter ) Block
                //
        case 330: {
            //#line 766 "GJavaParser.g"
            setResult(
                //#line 766 GJavaParser.g
                ast_pool.data.Next() = new CatchClause(getLeftIToken(), getRightIToken(),
                    //#line 766 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 766 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 766 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 766 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 766 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 766 GJavaParser.g
            );
            break;
        }
                //
                // Rule 331:  Finally ::= finally Block
                //
        case 331: {
            //#line 768 "GJavaParser.g"
            setResult(
                //#line 768 GJavaParser.g
                ast_pool.data.Next() = new Finally(getLeftIToken(), getRightIToken(),
                    //#line 768 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 768 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 768 GJavaParser.g
            );
            break;
        }
                //
                // Rule 332:  Primary ::= PrimaryNoNewArray
                //
        case 332:
            break;
            //
            // Rule 333:  Primary ::= ArrayCreationExpression
            //
        case 333:
            break;
            //
            // Rule 334:  PrimaryNoNewArray ::= Literal
            //
        case 334:
            break;
            //
            // Rule 335:  PrimaryNoNewArray ::= Type . class
            //
        case 335: {
            //#line 784 "GJavaParser.g"
            setResult(
                //#line 784 GJavaParser.g
                ast_pool.data.Next() = new PrimaryNoNewArray0(getLeftIToken(), getRightIToken(),
                    //#line 784 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 784 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 784 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 784 GJavaParser.g
            );
            break;
        }
                //
                // Rule 336:  PrimaryNoNewArray ::= void . class
                //
        case 336: {
            //#line 785 "GJavaParser.g"
            setResult(
                //#line 785 GJavaParser.g
                ast_pool.data.Next() = new PrimaryNoNewArray1(getLeftIToken(), getRightIToken(),
                    //#line 785 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 785 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 785 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 785 GJavaParser.g
            );
            break;
        }
                //
                // Rule 337:  PrimaryNoNewArray ::= this
                //
        case 337: {
            //#line 786 "GJavaParser.g"
            setResult(
                //#line 786 GJavaParser.g
                ast_pool.data.Next() = new PrimaryNoNewArray2(getRhsIToken(1))
                //#line 786 GJavaParser.g
            );
            break;
        }
                //
                // Rule 338:  PrimaryNoNewArray ::= ClassName . this
                //
        case 338: {
            //#line 787 "GJavaParser.g"
            setResult(
                //#line 787 GJavaParser.g
                ast_pool.data.Next() = new PrimaryNoNewArray3(getLeftIToken(), getRightIToken(),
                    //#line 787 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 787 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 787 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 787 GJavaParser.g
            );
            break;
        }
                //
                // Rule 339:  PrimaryNoNewArray ::= ( Expression )
                //
        case 339: {
            //#line 788 "GJavaParser.g"
            setResult(
                //#line 788 GJavaParser.g
                ast_pool.data.Next() = new PrimaryNoNewArray4(getLeftIToken(), getRightIToken(),
                    //#line 788 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 788 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 788 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 788 GJavaParser.g
            );
            break;
        }
                //
                // Rule 340:  PrimaryNoNewArray ::= ClassInstanceCreationExpression
                //
        case 340:
            break;
            //
            // Rule 341:  PrimaryNoNewArray ::= FieldAccess
            //
        case 341:
            break;
            //
            // Rule 342:  PrimaryNoNewArray ::= MethodInvocation
            //
        case 342:
            break;
            //
            // Rule 343:  PrimaryNoNewArray ::= ArrayAccess
            //
        case 343:
            break;
            //
            // Rule 344:  Literal ::= IntegerLiteral
            //
        case 344: {
            //#line 794 "GJavaParser.g"
            setResult(
                //#line 794 GJavaParser.g
                ast_pool.data.Next() = new Literal0(getRhsIToken(1))
                //#line 794 GJavaParser.g
            );
            break;
        }
                //
                // Rule 345:  Literal ::= LongLiteral
                //
        case 345: {
            //#line 795 "GJavaParser.g"
            setResult(
                //#line 795 GJavaParser.g
                ast_pool.data.Next() = new Literal1(getRhsIToken(1))
                //#line 795 GJavaParser.g
            );
            break;
        }
                //
                // Rule 346:  Literal ::= FloatingPointLiteral
                //
        case 346: {
            //#line 796 "GJavaParser.g"
            setResult(
                //#line 796 GJavaParser.g
                ast_pool.data.Next() = new Literal2(getRhsIToken(1))
                //#line 796 GJavaParser.g
            );
            break;
        }
                //
                // Rule 347:  Literal ::= DoubleLiteral
                //
        case 347: {
            //#line 797 "GJavaParser.g"
            setResult(
                //#line 797 GJavaParser.g
                ast_pool.data.Next() = new Literal3(getRhsIToken(1))
                //#line 797 GJavaParser.g
            );
            break;
        }
                //
                // Rule 348:  Literal ::= BooleanLiteral
                //
        case 348:
            break;
            //
            // Rule 349:  Literal ::= CharacterLiteral
            //
        case 349: {
            //#line 799 "GJavaParser.g"
            setResult(
                //#line 799 GJavaParser.g
                ast_pool.data.Next() = new Literal4(getRhsIToken(1))
                //#line 799 GJavaParser.g
            );
            break;
        }
                //
                // Rule 350:  Literal ::= StringLiteral
                //
        case 350: {
            //#line 800 "GJavaParser.g"
            setResult(
                //#line 800 GJavaParser.g
                ast_pool.data.Next() = new Literal5(getRhsIToken(1))
                //#line 800 GJavaParser.g
            );
            break;
        }
                //
                // Rule 351:  Literal ::= null
                //
        case 351: {
            //#line 801 "GJavaParser.g"
            setResult(
                //#line 801 GJavaParser.g
                ast_pool.data.Next() = new Literal6(getRhsIToken(1))
                //#line 801 GJavaParser.g
            );
            break;
        }
                //
                // Rule 352:  BooleanLiteral ::= true
                //
        case 352: {
            //#line 803 "GJavaParser.g"
            setResult(
                //#line 803 GJavaParser.g
                ast_pool.data.Next() = new BooleanLiteral0(getRhsIToken(1))
                //#line 803 GJavaParser.g
            );
            break;
        }
                //
                // Rule 353:  BooleanLiteral ::= false
                //
        case 353: {
            //#line 804 "GJavaParser.g"
            setResult(
                //#line 804 GJavaParser.g
                ast_pool.data.Next() = new BooleanLiteral1(getRhsIToken(1))
                //#line 804 GJavaParser.g
            );
            break;
        }
                //
                // Rule 354:  ClassInstanceCreationExpression ::= new TypeArgumentsopt ClassOrInterfaceType TypeArgumentsopt ( ArgumentListopt ) ClassBodyopt
                //
        case 354: {
            //#line 811 "GJavaParser.g"
            setResult(
                //#line 811 GJavaParser.g
                ast_pool.data.Next() = new ClassInstanceCreationExpression0(getLeftIToken(), getRightIToken(),
                    //#line 811 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 811 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 811 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 811 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 811 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 811 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 811 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)),
                    //#line 811 GJavaParser.g
                    (IAst*)getRhsSym(8))
                //#line 811 GJavaParser.g
            );
            break;
        }
                //
                // Rule 355:  ClassInstanceCreationExpression ::= Primary . new TypeArgumentsopt identifier TypeArgumentsopt ( ArgumentListopt ) ClassBodyopt
                //
        case 355: {
            //#line 812 "GJavaParser.g"
            setResult(
                //#line 812 GJavaParser.g
                ast_pool.data.Next() = new ClassInstanceCreationExpression1(getLeftIToken(), getRightIToken(),
                    //#line 812 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 812 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 812 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 812 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 812 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 812 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 812 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)),
                    //#line 812 GJavaParser.g
                    (IAst*)getRhsSym(8),
                    //#line 812 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(9)),
                    //#line 812 GJavaParser.g
                    (IAst*)getRhsSym(10))
                //#line 812 GJavaParser.g
            );
            break;
        }
                //
                // Rule 356:  ArgumentList ::= Expression
                //
        case 356:
            break;
            //
            // Rule 357:  ArgumentList ::= ArgumentList , Expression
            //
        case 357: {
            //#line 816 "GJavaParser.g"
            setResult(
                //#line 816 GJavaParser.g
                ast_pool.data.Next() = new ArgumentList(getLeftIToken(), getRightIToken(),
                    //#line 816 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 816 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 816 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 816 GJavaParser.g
            );
            break;
        }
                //
                // Rule 358:  ArrayCreationExpression ::= new PrimitiveType DimExprs Dimsopt
                //
        case 358: {
            //#line 826 "GJavaParser.g"
            setResult(
                //#line 826 GJavaParser.g
                ast_pool.data.Next() = new ArrayCreationExpression0(getLeftIToken(), getRightIToken(),
                    //#line 826 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 826 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 826 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 826 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 826 GJavaParser.g
            );
            break;
        }
                //
                // Rule 359:  ArrayCreationExpression ::= new ClassOrInterfaceType DimExprs Dimsopt
                //
        case 359: {
            //#line 827 "GJavaParser.g"
            setResult(
                //#line 827 GJavaParser.g
                ast_pool.data.Next() = new ArrayCreationExpression1(getLeftIToken(), getRightIToken(),
                    //#line 827 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 827 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 827 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 827 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 827 GJavaParser.g
            );
            break;
        }
                //
                // Rule 360:  ArrayCreationExpression ::= new PrimitiveType Dims ArrayInitializer
                //
        case 360: {
            //#line 828 "GJavaParser.g"
            setResult(
                //#line 828 GJavaParser.g
                ast_pool.data.Next() = new ArrayCreationExpression2(getLeftIToken(), getRightIToken(),
                    //#line 828 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 828 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 828 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 828 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 828 GJavaParser.g
            );
            break;
        }
                //
                // Rule 361:  ArrayCreationExpression ::= new ClassOrInterfaceType Dims ArrayInitializer
                //
        case 361: {
            //#line 829 "GJavaParser.g"
            setResult(
                //#line 829 GJavaParser.g
                ast_pool.data.Next() = new ArrayCreationExpression3(getLeftIToken(), getRightIToken(),
                    //#line 829 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 829 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 829 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 829 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 829 GJavaParser.g
            );
            break;
        }
                //
                // Rule 362:  DimExprs ::= DimExpr
                //
        case 362:
            break;
            //
            // Rule 363:  DimExprs ::= DimExprs DimExpr
            //
        case 363: {
            //#line 832 "GJavaParser.g"
            setResult(
                //#line 832 GJavaParser.g
                ast_pool.data.Next() = new DimExprs(getLeftIToken(), getRightIToken(),
                    //#line 832 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 832 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 832 GJavaParser.g
            );
            break;
        }
                //
                // Rule 364:  DimExpr ::= [ Expression ]
                //
        case 364: {
            //#line 834 "GJavaParser.g"
            setResult(
                //#line 834 GJavaParser.g
                ast_pool.data.Next() = new DimExpr(getLeftIToken(), getRightIToken(),
                    //#line 834 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 834 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 834 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 834 GJavaParser.g
            );
            break;
        }
                //
                // Rule 365:  Dims ::= [ ]
                //
        case 365: {
            //#line 836 "GJavaParser.g"
            setResult(
                //#line 836 GJavaParser.g
                ast_pool.data.Next() = new Dims0(getLeftIToken(), getRightIToken(),
                    //#line 836 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 836 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)))
                //#line 836 GJavaParser.g
            );
            break;
        }
                //
                // Rule 366:  Dims ::= Dims [ ]
                //
        case 366: {
            //#line 837 "GJavaParser.g"
            setResult(
                //#line 837 GJavaParser.g
                ast_pool.data.Next() = new Dims1(getLeftIToken(), getRightIToken(),
                    //#line 837 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 837 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 837 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 837 GJavaParser.g
            );
            break;
        }
                //
                // Rule 367:  FieldAccess ::= Primary . identifier
                //
        case 367: {
            //#line 839 "GJavaParser.g"
            setResult(
                //#line 839 GJavaParser.g
                ast_pool.data.Next() = new FieldAccess0(getLeftIToken(), getRightIToken(),
                    //#line 839 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 839 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 839 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 839 GJavaParser.g
            );
            break;
        }
                //
                // Rule 368:  FieldAccess ::= super . identifier
                //
        case 368: {
            //#line 840 "GJavaParser.g"
            setResult(
                //#line 840 GJavaParser.g
                ast_pool.data.Next() = new FieldAccess1(getLeftIToken(), getRightIToken(),
                    //#line 840 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 840 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 840 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 840 GJavaParser.g
            );
            break;
        }
                //
                // Rule 369:  FieldAccess ::= ClassName . super . identifier
                //
        case 369: {
            //#line 841 "GJavaParser.g"
            setResult(
                //#line 841 GJavaParser.g
                ast_pool.data.Next() = new FieldAccess2(getLeftIToken(), getRightIToken(),
                    //#line 841 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 841 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 841 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 841 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 841 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 841 GJavaParser.g
            );
            break;
        }
                //
                // Rule 370:  MethodInvocation ::= MethodName ( ArgumentListopt )
                //
        case 370: {
            //#line 843 "GJavaParser.g"
            setResult(
                //#line 843 GJavaParser.g
                ast_pool.data.Next() = new MethodInvocation0(getLeftIToken(), getRightIToken(),
                    //#line 843 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 843 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 843 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 843 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 843 GJavaParser.g
            );
            break;
        }
                //
                // Rule 371:  MethodInvocation ::= Primary . TypeArgumentsopt identifier ( ArgumentListopt )
                //
        case 371: {
            //#line 844 "GJavaParser.g"
            setResult(
                //#line 844 GJavaParser.g
                ast_pool.data.Next() = new MethodInvocation1(getLeftIToken(), getRightIToken(),
                    //#line 844 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 844 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 844 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 844 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 844 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 844 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 844 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)))
                //#line 844 GJavaParser.g
            );
            break;
        }
                //
                // Rule 372:  MethodInvocation ::= super . TypeArgumentsopt identifier ( ArgumentListopt )
                //
        case 372: {
            //#line 845 "GJavaParser.g"
            setResult(
                //#line 845 GJavaParser.g
                ast_pool.data.Next() = new MethodInvocation2(getLeftIToken(), getRightIToken(),
                    //#line 845 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 845 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 845 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 845 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 845 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 845 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 845 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)))
                //#line 845 GJavaParser.g
            );
            break;
        }
                //
                // Rule 373:  MethodInvocation ::= ClassName . super . TypeArgumentsopt identifier ( ArgumentListopt )
                //
        case 373: {
            //#line 846 "GJavaParser.g"
            setResult(
                //#line 846 GJavaParser.g
                ast_pool.data.Next() = new MethodInvocation3(getLeftIToken(), getRightIToken(),
                    //#line 846 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 846 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 846 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 846 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 846 GJavaParser.g
                    (IAst*)getRhsSym(5),
                    //#line 846 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 846 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)),
                    //#line 846 GJavaParser.g
                    (IAst*)getRhsSym(8),
                    //#line 846 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(9)))
                //#line 846 GJavaParser.g
            );
            break;
        }
                //
                // Rule 374:  MethodInvocation ::= TypeName . TypeArguments identifier ( ArgumentListopt )
                //
        case 374: {
            //#line 847 "GJavaParser.g"
            setResult(
                //#line 847 GJavaParser.g
                ast_pool.data.Next() = new MethodInvocation4(getLeftIToken(), getRightIToken(),
                    //#line 847 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 847 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 847 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 847 GJavaParser.g
                    (IAst*)getRhsSym(4),
                    //#line 847 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(5)),
                    //#line 847 GJavaParser.g
                    (IAst*)getRhsSym(6),
                    //#line 847 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(7)))
                //#line 847 GJavaParser.g
            );
            break;
        }
                //
                // Rule 375:  ArrayAccess ::= ExpressionName [ Expression ]
                //
        case 375: {
            //#line 855 "GJavaParser.g"
            setResult(
                //#line 855 GJavaParser.g
                ast_pool.data.Next() = new ArrayAccess0(getLeftIToken(), getRightIToken(),
                    //#line 855 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 855 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 855 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 855 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 855 GJavaParser.g
            );
            break;
        }
                //
                // Rule 376:  ArrayAccess ::= PrimaryNoNewArray [ Expression ]
                //
        case 376: {
            //#line 856 "GJavaParser.g"
            setResult(
                //#line 856 GJavaParser.g
                ast_pool.data.Next() = new ArrayAccess1(getLeftIToken(), getRightIToken(),
                    //#line 856 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 856 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 856 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 856 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 856 GJavaParser.g
            );
            break;
        }
                //
                // Rule 377:  PostfixExpression ::= Primary
                //
        case 377:
            break;
            //
            // Rule 378:  PostfixExpression ::= ExpressionName
            //
        case 378:
            break;
            //
            // Rule 379:  PostfixExpression ::= PostIncrementExpression
            //
        case 379:
            break;
            //
            // Rule 380:  PostfixExpression ::= PostDecrementExpression
            //
        case 380:
            break;
            //
            // Rule 381:  PostIncrementExpression ::= PostfixExpression ++
            //
        case 381: {
            //#line 863 "GJavaParser.g"
            setResult(
                //#line 863 GJavaParser.g
                ast_pool.data.Next() = new PostIncrementExpression(getLeftIToken(), getRightIToken(),
                    //#line 863 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 863 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)))
                //#line 863 GJavaParser.g
            );
            break;
        }
                //
                // Rule 382:  PostDecrementExpression ::= PostfixExpression --
                //
        case 382: {
            //#line 865 "GJavaParser.g"
            setResult(
                //#line 865 GJavaParser.g
                ast_pool.data.Next() = new PostDecrementExpression(getLeftIToken(), getRightIToken(),
                    //#line 865 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 865 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)))
                //#line 865 GJavaParser.g
            );
            break;
        }
                //
                // Rule 383:  UnaryExpression ::= PreIncrementExpression
                //
        case 383:
            break;
            //
            // Rule 384:  UnaryExpression ::= PreDecrementExpression
            //
        case 384:
            break;
            //
            // Rule 385:  UnaryExpression ::= + UnaryExpression
            //
        case 385: {
            //#line 869 "GJavaParser.g"
            setResult(
                //#line 869 GJavaParser.g
                ast_pool.data.Next() = new UnaryExpression0(getLeftIToken(), getRightIToken(),
                    //#line 869 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 869 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 869 GJavaParser.g
            );
            break;
        }
                //
                // Rule 386:  UnaryExpression ::= - UnaryExpression
                //
        case 386: {
            //#line 870 "GJavaParser.g"
            setResult(
                //#line 870 GJavaParser.g
                ast_pool.data.Next() = new UnaryExpression1(getLeftIToken(), getRightIToken(),
                    //#line 870 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 870 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 870 GJavaParser.g
            );
            break;
        }
                //
                // Rule 387:  UnaryExpression ::= UnaryExpressionNotPlusMinus
                //
        case 387:
            break;
            //
            // Rule 388:  PreIncrementExpression ::= ++ UnaryExpression
            //
        case 388: {
            //#line 873 "GJavaParser.g"
            setResult(
                //#line 873 GJavaParser.g
                ast_pool.data.Next() = new PreIncrementExpression(getLeftIToken(), getRightIToken(),
                    //#line 873 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 873 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 873 GJavaParser.g
            );
            break;
        }
                //
                // Rule 389:  PreDecrementExpression ::= -- UnaryExpression
                //
        case 389: {
            //#line 875 "GJavaParser.g"
            setResult(
                //#line 875 GJavaParser.g
                ast_pool.data.Next() = new PreDecrementExpression(getLeftIToken(), getRightIToken(),
                    //#line 875 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 875 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 875 GJavaParser.g
            );
            break;
        }
                //
                // Rule 390:  UnaryExpressionNotPlusMinus ::= PostfixExpression
                //
        case 390:
            break;
            //
            // Rule 391:  UnaryExpressionNotPlusMinus ::= ~ UnaryExpression
            //
        case 391: {
            //#line 878 "GJavaParser.g"
            setResult(
                //#line 878 GJavaParser.g
                ast_pool.data.Next() = new UnaryExpressionNotPlusMinus0(getLeftIToken(), getRightIToken(),
                    //#line 878 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 878 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 878 GJavaParser.g
            );
            break;
        }
                //
                // Rule 392:  UnaryExpressionNotPlusMinus ::= ! UnaryExpression
                //
        case 392: {
            //#line 879 "GJavaParser.g"
            setResult(
                //#line 879 GJavaParser.g
                ast_pool.data.Next() = new UnaryExpressionNotPlusMinus1(getLeftIToken(), getRightIToken(),
                    //#line 879 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 879 GJavaParser.g
                    (IAst*)getRhsSym(2))
                //#line 879 GJavaParser.g
            );
            break;
        }
                //
                // Rule 393:  UnaryExpressionNotPlusMinus ::= CastExpression
                //
        case 393:
            break;
            //
            // Rule 394:  CastExpression ::= ( PrimitiveType Dimsopt ) UnaryExpression
            //
        case 394: {
            //#line 882 "GJavaParser.g"
            setResult(
                //#line 882 GJavaParser.g
                ast_pool.data.Next() = new CastExpression0(getLeftIToken(), getRightIToken(),
                    //#line 882 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 882 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 882 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 882 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 882 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 882 GJavaParser.g
            );
            break;
        }
                //
                // Rule 395:  CastExpression ::= ( ReferenceType ) UnaryExpressionNotPlusMinus
                //
        case 395: {
            //#line 883 "GJavaParser.g"
            setResult(
                //#line 883 GJavaParser.g
                ast_pool.data.Next() = new CastExpression1(getLeftIToken(), getRightIToken(),
                    //#line 883 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 883 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 883 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 883 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 883 GJavaParser.g
            );
            break;
        }
                //
                // Rule 396:  MultiplicativeExpression ::= UnaryExpression
                //
        case 396:
            break;
            //
            // Rule 397:  MultiplicativeExpression ::= MultiplicativeExpression * UnaryExpression
            //
        case 397: {
            //#line 886 "GJavaParser.g"
            setResult(
                //#line 886 GJavaParser.g
                ast_pool.data.Next() = new MultiplicativeExpression0(getLeftIToken(), getRightIToken(),
                    //#line 886 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 886 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 886 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 886 GJavaParser.g
            );
            break;
        }
                //
                // Rule 398:  MultiplicativeExpression ::= MultiplicativeExpression / UnaryExpression
                //
        case 398: {
            //#line 887 "GJavaParser.g"
            setResult(
                //#line 887 GJavaParser.g
                ast_pool.data.Next() = new MultiplicativeExpression1(getLeftIToken(), getRightIToken(),
                    //#line 887 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 887 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 887 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 887 GJavaParser.g
            );
            break;
        }
                //
                // Rule 399:  MultiplicativeExpression ::= MultiplicativeExpression % UnaryExpression
                //
        case 399: {
            //#line 888 "GJavaParser.g"
            setResult(
                //#line 888 GJavaParser.g
                ast_pool.data.Next() = new MultiplicativeExpression2(getLeftIToken(), getRightIToken(),
                    //#line 888 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 888 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 888 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 888 GJavaParser.g
            );
            break;
        }
                //
                // Rule 400:  AdditiveExpression ::= MultiplicativeExpression
                //
        case 400:
            break;
            //
            // Rule 401:  AdditiveExpression ::= AdditiveExpression + MultiplicativeExpression
            //
        case 401: {
            //#line 891 "GJavaParser.g"
            setResult(
                //#line 891 GJavaParser.g
                ast_pool.data.Next() = new AdditiveExpression0(getLeftIToken(), getRightIToken(),
                    //#line 891 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 891 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 891 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 891 GJavaParser.g
            );
            break;
        }
                //
                // Rule 402:  AdditiveExpression ::= AdditiveExpression - MultiplicativeExpression
                //
        case 402: {
            //#line 892 "GJavaParser.g"
            setResult(
                //#line 892 GJavaParser.g
                ast_pool.data.Next() = new AdditiveExpression1(getLeftIToken(), getRightIToken(),
                    //#line 892 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 892 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 892 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 892 GJavaParser.g
            );
            break;
        }
                //
                // Rule 403:  ShiftExpression ::= AdditiveExpression
                //
        case 403:
            break;
            //
            // Rule 404:  ShiftExpression ::= ShiftExpression << AdditiveExpression
            //
        case 404: {
            //#line 895 "GJavaParser.g"
            setResult(
                //#line 895 GJavaParser.g
                ast_pool.data.Next() = new ShiftExpression0(getLeftIToken(), getRightIToken(),
                    //#line 895 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 895 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 895 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 895 GJavaParser.g
            );
            break;
        }
                //
                // Rule 405:  ShiftExpression ::= ShiftExpression > > AdditiveExpression
                //
        case 405: {
            //#line 896 "GJavaParser.g"
            setResult(
                //#line 896 GJavaParser.g
                ast_pool.data.Next() = new ShiftExpression1(getLeftIToken(), getRightIToken(),
                    //#line 896 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 896 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 896 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 896 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 896 GJavaParser.g
            );
            break;
        }
                //
                // Rule 406:  ShiftExpression ::= ShiftExpression > > > AdditiveExpression
                //
        case 406: {
            //#line 897 "GJavaParser.g"
            setResult(
                //#line 897 GJavaParser.g
                ast_pool.data.Next() = new ShiftExpression2(getLeftIToken(), getRightIToken(),
                    //#line 897 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 897 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 897 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 897 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 897 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 897 GJavaParser.g
            );
            break;
        }
                //
                // Rule 407:  RelationalExpression ::= ShiftExpression
                //
        case 407:
            break;
            //
            // Rule 408:  RelationalExpression ::= RelationalExpression < ShiftExpression
            //
        case 408: {
            //#line 900 "GJavaParser.g"
            setResult(
                //#line 900 GJavaParser.g
                ast_pool.data.Next() = new RelationalExpression0(getLeftIToken(), getRightIToken(),
                    //#line 900 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 900 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 900 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 900 GJavaParser.g
            );
            break;
        }
                //
                // Rule 409:  RelationalExpression ::= RelationalExpression > ShiftExpression
                //
        case 409: {
            //#line 901 "GJavaParser.g"
            setResult(
                //#line 901 GJavaParser.g
                ast_pool.data.Next() = new RelationalExpression1(getLeftIToken(), getRightIToken(),
                    //#line 901 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 901 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 901 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 901 GJavaParser.g
            );
            break;
        }
                //
                // Rule 410:  RelationalExpression ::= RelationalExpression <= ShiftExpression
                //
        case 410: {
            //#line 902 "GJavaParser.g"
            setResult(
                //#line 902 GJavaParser.g
                ast_pool.data.Next() = new RelationalExpression2(getLeftIToken(), getRightIToken(),
                    //#line 902 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 902 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 902 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 902 GJavaParser.g
            );
            break;
        }
                //
                // Rule 411:  RelationalExpression ::= RelationalExpression > = ShiftExpression
                //
        case 411: {
            //#line 903 "GJavaParser.g"
            setResult(
                //#line 903 GJavaParser.g
                ast_pool.data.Next() = new RelationalExpression3(getLeftIToken(), getRightIToken(),
                    //#line 903 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 903 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 903 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 903 GJavaParser.g
                    (IAst*)getRhsSym(4))
                //#line 903 GJavaParser.g
            );
            break;
        }
                //
                // Rule 412:  RelationalExpression ::= RelationalExpression instanceof ReferenceType
                //
        case 412: {
            //#line 904 "GJavaParser.g"
            setResult(
                //#line 904 GJavaParser.g
                ast_pool.data.Next() = new RelationalExpression4(getLeftIToken(), getRightIToken(),
                    //#line 904 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 904 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 904 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 904 GJavaParser.g
            );
            break;
        }
                //
                // Rule 413:  EqualityExpression ::= RelationalExpression
                //
        case 413:
            break;
            //
            // Rule 414:  EqualityExpression ::= EqualityExpression == RelationalExpression
            //
        case 414: {
            //#line 907 "GJavaParser.g"
            setResult(
                //#line 907 GJavaParser.g
                ast_pool.data.Next() = new EqualityExpression0(getLeftIToken(), getRightIToken(),
                    //#line 907 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 907 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 907 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 907 GJavaParser.g
            );
            break;
        }
                //
                // Rule 415:  EqualityExpression ::= EqualityExpression != RelationalExpression
                //
        case 415: {
            //#line 908 "GJavaParser.g"
            setResult(
                //#line 908 GJavaParser.g
                ast_pool.data.Next() = new EqualityExpression1(getLeftIToken(), getRightIToken(),
                    //#line 908 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 908 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 908 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 908 GJavaParser.g
            );
            break;
        }
                //
                // Rule 416:  AndExpression ::= EqualityExpression
                //
        case 416:
            break;
            //
            // Rule 417:  AndExpression ::= AndExpression & EqualityExpression
            //
        case 417: {
            //#line 911 "GJavaParser.g"
            setResult(
                //#line 911 GJavaParser.g
                ast_pool.data.Next() = new AndExpression(getLeftIToken(), getRightIToken(),
                    //#line 911 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 911 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 911 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 911 GJavaParser.g
            );
            break;
        }
                //
                // Rule 418:  ExclusiveOrExpression ::= AndExpression
                //
        case 418:
            break;
            //
            // Rule 419:  ExclusiveOrExpression ::= ExclusiveOrExpression ^ AndExpression
            //
        case 419: {
            //#line 914 "GJavaParser.g"
            setResult(
                //#line 914 GJavaParser.g
                ast_pool.data.Next() = new ExclusiveOrExpression(getLeftIToken(), getRightIToken(),
                    //#line 914 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 914 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 914 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 914 GJavaParser.g
            );
            break;
        }
                //
                // Rule 420:  InclusiveOrExpression ::= ExclusiveOrExpression
                //
        case 420:
            break;
            //
            // Rule 421:  InclusiveOrExpression ::= InclusiveOrExpression | ExclusiveOrExpression
            //
        case 421: {
            //#line 917 "GJavaParser.g"
            setResult(
                //#line 917 GJavaParser.g
                ast_pool.data.Next() = new InclusiveOrExpression(getLeftIToken(), getRightIToken(),
                    //#line 917 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 917 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 917 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 917 GJavaParser.g
            );
            break;
        }
                //
                // Rule 422:  ConditionalAndExpression ::= InclusiveOrExpression
                //
        case 422:
            break;
            //
            // Rule 423:  ConditionalAndExpression ::= ConditionalAndExpression && InclusiveOrExpression
            //
        case 423: {
            //#line 920 "GJavaParser.g"
            setResult(
                //#line 920 GJavaParser.g
                ast_pool.data.Next() = new ConditionalAndExpression(getLeftIToken(), getRightIToken(),
                    //#line 920 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 920 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 920 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 920 GJavaParser.g
            );
            break;
        }
                //
                // Rule 424:  ConditionalOrExpression ::= ConditionalAndExpression
                //
        case 424:
            break;
            //
            // Rule 425:  ConditionalOrExpression ::= ConditionalOrExpression || ConditionalAndExpression
            //
        case 425: {
            //#line 923 "GJavaParser.g"
            setResult(
                //#line 923 GJavaParser.g
                ast_pool.data.Next() = new ConditionalOrExpression(getLeftIToken(), getRightIToken(),
                    //#line 923 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 923 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 923 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 923 GJavaParser.g
            );
            break;
        }
                //
                // Rule 426:  ConditionalExpression ::= ConditionalOrExpression
                //
        case 426:
            break;
            //
            // Rule 427:  ConditionalExpression ::= ConditionalOrExpression ? Expression : ConditionalExpression
            //
        case 427: {
            //#line 926 "GJavaParser.g"
            setResult(
                //#line 926 GJavaParser.g
                ast_pool.data.Next() = new ConditionalExpression(getLeftIToken(), getRightIToken(),
                    //#line 926 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 926 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 926 GJavaParser.g
                    (IAst*)getRhsSym(3),
                    //#line 926 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)),
                    //#line 926 GJavaParser.g
                    (IAst*)getRhsSym(5))
                //#line 926 GJavaParser.g
            );
            break;
        }
                //
                // Rule 428:  AssignmentExpression ::= ConditionalExpression
                //
        case 428:
            break;
            //
            // Rule 429:  AssignmentExpression ::= Assignment
            //
        case 429:
            break;
            //
            // Rule 430:  Assignment ::= LeftHandSide AssignmentOperator AssignmentExpression
            //
        case 430: {
            //#line 931 "GJavaParser.g"
            setResult(
                //#line 931 GJavaParser.g
                ast_pool.data.Next() = new Assignment(getLeftIToken(), getRightIToken(),
                    //#line 931 GJavaParser.g
                    (IAst*)getRhsSym(1),
                    //#line 931 GJavaParser.g
                    (IAst*)getRhsSym(2),
                    //#line 931 GJavaParser.g
                    (IAst*)getRhsSym(3))
                //#line 931 GJavaParser.g
            );
            break;
        }
                //
                // Rule 431:  LeftHandSide ::= ExpressionName
                //
        case 431:
            break;
            //
            // Rule 432:  LeftHandSide ::= FieldAccess
            //
        case 432:
            break;
            //
            // Rule 433:  LeftHandSide ::= ArrayAccess
            //
        case 433:
            break;
            //
            // Rule 434:  AssignmentOperator ::= =
            //
        case 434: {
            //#line 937 "GJavaParser.g"
            setResult(
                //#line 937 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator0(getRhsIToken(1))
                //#line 937 GJavaParser.g
            );
            break;
        }
                //
                // Rule 435:  AssignmentOperator ::= *=
                //
        case 435: {
            //#line 938 "GJavaParser.g"
            setResult(
                //#line 938 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator1(getRhsIToken(1))
                //#line 938 GJavaParser.g
            );
            break;
        }
                //
                // Rule 436:  AssignmentOperator ::= /=
                //
        case 436: {
            //#line 939 "GJavaParser.g"
            setResult(
                //#line 939 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator2(getRhsIToken(1))
                //#line 939 GJavaParser.g
            );
            break;
        }
                //
                // Rule 437:  AssignmentOperator ::= %=
                //
        case 437: {
            //#line 940 "GJavaParser.g"
            setResult(
                //#line 940 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator3(getRhsIToken(1))
                //#line 940 GJavaParser.g
            );
            break;
        }
                //
                // Rule 438:  AssignmentOperator ::= +=
                //
        case 438: {
            //#line 941 "GJavaParser.g"
            setResult(
                //#line 941 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator4(getRhsIToken(1))
                //#line 941 GJavaParser.g
            );
            break;
        }
                //
                // Rule 439:  AssignmentOperator ::= -=
                //
        case 439: {
            //#line 942 "GJavaParser.g"
            setResult(
                //#line 942 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator5(getRhsIToken(1))
                //#line 942 GJavaParser.g
            );
            break;
        }
                //
                // Rule 440:  AssignmentOperator ::= <<=
                //
        case 440: {
            //#line 943 "GJavaParser.g"
            setResult(
                //#line 943 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator6(getRhsIToken(1))
                //#line 943 GJavaParser.g
            );
            break;
        }
                //
                // Rule 441:  AssignmentOperator ::= > > =
                //
        case 441: {
            //#line 944 "GJavaParser.g"
            setResult(
                //#line 944 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator7(getLeftIToken(), getRightIToken(),
                    //#line 944 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 944 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 944 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)))
                //#line 944 GJavaParser.g
            );
            break;
        }
                //
                // Rule 442:  AssignmentOperator ::= > > > =
                //
        case 442: {
            //#line 945 "GJavaParser.g"
            setResult(
                //#line 945 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator8(getLeftIToken(), getRightIToken(),
                    //#line 945 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(1)),
                    //#line 945 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(2)),
                    //#line 945 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(3)),
                    //#line 945 GJavaParser.g
                    ast_pool.data.Next() = new AstToken(getRhsIToken(4)))
                //#line 945 GJavaParser.g
            );
            break;
        }
                //
                // Rule 443:  AssignmentOperator ::= &=
                //
        case 443: {
            //#line 946 "GJavaParser.g"
            setResult(
                //#line 946 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator9(getRhsIToken(1))
                //#line 946 GJavaParser.g
            );
            break;
        }
                //
                // Rule 444:  AssignmentOperator ::= ^=
                //
        case 444: {
            //#line 947 "GJavaParser.g"
            setResult(
                //#line 947 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator10(getRhsIToken(1))
                //#line 947 GJavaParser.g
            );
            break;
        }
                //
                // Rule 445:  AssignmentOperator ::= |=
                //
        case 445: {
            //#line 948 "GJavaParser.g"
            setResult(
                //#line 948 GJavaParser.g
                ast_pool.data.Next() = new AssignmentOperator11(getRhsIToken(1))
                //#line 948 GJavaParser.g
            );
            break;
        }
                //
                // Rule 446:  Expression ::= AssignmentExpression
                //
        case 446:
            break;
            //
            // Rule 447:  ConstantExpression ::= Expression
            //
        case 447:
            break;
            //
            // Rule 448:  Dimsopt ::= $Empty
            //
        case 448: {
            //#line 957 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 449:  Dimsopt ::= Dims
                //
        case 449:
            break;
            //
            // Rule 450:  Catchesopt ::= $Empty
            //
        case 450: {
            //#line 960 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 451:  Catchesopt ::= Catches
                //
        case 451:
            break;
            //
            // Rule 452:  identifieropt ::= $Empty
            //
        case 452: {
            //#line 963 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 453:  identifieropt ::= identifier
                //
        case 453:
            break;
            //
            // Rule 454:  ForUpdateopt ::= $Empty
            //
        case 454: {
            //#line 966 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 455:  ForUpdateopt ::= ForUpdate
                //
        case 455:
            break;
            //
            // Rule 456:  Expressionopt ::= $Empty
            //
        case 456: {
            //#line 969 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 457:  Expressionopt ::= Expression
                //
        case 457:
            break;
            //
            // Rule 458:  ForInitopt ::= $Empty
            //
        case 458: {
            //#line 972 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 459:  ForInitopt ::= ForInit
                //
        case 459:
            break;
            //
            // Rule 460:  SwitchLabelsopt ::= $Empty
            //
        case 460: {
            //#line 975 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 461:  SwitchLabelsopt ::= SwitchLabels
                //
        case 461:
            break;
            //
            // Rule 462:  SwitchBlockStatementGroupsopt ::= $Empty
            //
        case 462: {
            //#line 978 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 463:  SwitchBlockStatementGroupsopt ::= SwitchBlockStatementGroups
                //
        case 463:
            break;
            //
            // Rule 464:  VariableModifiersopt ::= $Empty
            //
        case 464: {
            //#line 981 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 465:  VariableModifiersopt ::= VariableModifiers
                //
        case 465:
            break;
            //
            // Rule 466:  VariableInitializersopt ::= $Empty
            //
        case 466: {
            //#line 984 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 467:  VariableInitializersopt ::= VariableInitializers
                //
        case 467:
            break;
            //
            // Rule 468:  ElementValuesopt ::= $Empty
            //
        case 468: {
            //#line 987 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 469:  ElementValuesopt ::= ElementValues
                //
        case 469:
            break;
            //
            // Rule 470:  ElementValuePairsopt ::= $Empty
            //
        case 470: {
            //#line 990 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 471:  ElementValuePairsopt ::= ElementValuePairs
                //
        case 471:
            break;
            //
            // Rule 472:  DefaultValueopt ::= $Empty
            //
        case 472: {
            //#line 993 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 473:  DefaultValueopt ::= DefaultValue
                //
        case 473:
            break;
            //
            // Rule 474:  AnnotationTypeElementDeclarationsopt ::= $Empty
            //
        case 474: {
            //#line 996 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 475:  AnnotationTypeElementDeclarationsopt ::= AnnotationTypeElementDeclarations
                //
        case 475:
            break;
            //
            // Rule 476:  AbstractMethodModifiersopt ::= $Empty
            //
        case 476: {
            //#line 999 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 477:  AbstractMethodModifiersopt ::= AbstractMethodModifiers
                //
        case 477:
            break;
            //
            // Rule 478:  ConstantModifiersopt ::= $Empty
            //
        case 478: {
            //#line 1002 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 479:  ConstantModifiersopt ::= ConstantModifiers
                //
        case 479:
            break;
            //
            // Rule 480:  InterfaceMemberDeclarationsopt ::= $Empty
            //
        case 480: {
            //#line 1005 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 481:  InterfaceMemberDeclarationsopt ::= InterfaceMemberDeclarations
                //
        case 481:
            break;
            //
            // Rule 482:  ExtendsInterfacesopt ::= $Empty
            //
        case 482: {
            //#line 1008 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 483:  ExtendsInterfacesopt ::= ExtendsInterfaces
                //
        case 483:
            break;
            //
            // Rule 484:  InterfaceModifiersopt ::= $Empty
            //
        case 484: {
            //#line 1011 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 485:  InterfaceModifiersopt ::= InterfaceModifiers
                //
        case 485:
            break;
            //
            // Rule 486:  ClassBodyopt ::= $Empty
            //
        case 486: {
            //#line 1014 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 487:  ClassBodyopt ::= ClassBody
                //
        case 487:
            break;
            //
            // Rule 488:  Argumentsopt ::= $Empty
            //
        case 488: {
            //#line 1017 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 489:  Argumentsopt ::= Arguments
                //
        case 489:
            break;
            //
            // Rule 490:  EnumBodyDeclarationsopt ::= $Empty
            //
        case 490: {
            //#line 1020 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 491:  EnumBodyDeclarationsopt ::= EnumBodyDeclarations
                //
        case 491:
            break;
            //
            // Rule 492:  ,opt ::= $Empty
            //
        case 492: {
            //#line 1023 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 493:  ,opt ::= ,
                //
        case 493: {
            //#line 1024 "GJavaParser.g"
            setResult(
                //#line 1024 GJavaParser.g
                ast_pool.data.Next() = new Commaopt(getRhsIToken(1))
                //#line 1024 GJavaParser.g
            );
            break;
        }
                //
                // Rule 494:  EnumConstantsopt ::= $Empty
                //
        case 494: {
            //#line 1026 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 495:  EnumConstantsopt ::= EnumConstants
                //
        case 495:
            break;
            //
            // Rule 496:  ArgumentListopt ::= $Empty
            //
        case 496: {
            //#line 1029 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 497:  ArgumentListopt ::= ArgumentList
                //
        case 497:
            break;
            //
            // Rule 498:  BlockStatementsopt ::= $Empty
            //
        case 498: {
            //#line 1032 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 499:  BlockStatementsopt ::= BlockStatements
                //
        case 499:
            break;
            //
            // Rule 500:  ExplicitConstructorInvocationopt ::= $Empty
            //
        case 500: {
            //#line 1035 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 501:  ExplicitConstructorInvocationopt ::= ExplicitConstructorInvocation
                //
        case 501:
            break;
            //
            // Rule 502:  ConstructorModifiersopt ::= $Empty
            //
        case 502: {
            //#line 1038 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 503:  ConstructorModifiersopt ::= ConstructorModifiers
                //
        case 503:
            break;
            //
            // Rule 504:  ...opt ::= $Empty
            //
        case 504: {
            //#line 1041 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 505:  ...opt ::= ...
                //
        case 505: {
            //#line 1042 "GJavaParser.g"
            setResult(
                //#line 1042 GJavaParser.g
                ast_pool.data.Next() = new Ellipsisopt(getRhsIToken(1))
                //#line 1042 GJavaParser.g
            );
            break;
        }
                //
                // Rule 506:  FormalParameterListopt ::= $Empty
                //
        case 506: {
            //#line 1044 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 507:  FormalParameterListopt ::= FormalParameterList
                //
        case 507:
            break;
            //
            // Rule 508:  Throwsopt ::= $Empty
            //
        case 508: {
            //#line 1047 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 509:  Throwsopt ::= Throws
                //
        case 509:
            break;
            //
            // Rule 510:  MethodModifiersopt ::= $Empty
            //
        case 510: {
            //#line 1050 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 511:  MethodModifiersopt ::= MethodModifiers
                //
        case 511:
            break;
            //
            // Rule 512:  FieldModifiersopt ::= $Empty
            //
        case 512: {
            //#line 1053 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 513:  FieldModifiersopt ::= FieldModifiers
                //
        case 513:
            break;
            //
            // Rule 514:  ClassBodyDeclarationsopt ::= $Empty
            //
        case 514: {
            //#line 1056 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 515:  ClassBodyDeclarationsopt ::= ClassBodyDeclarations
                //
        case 515:
            break;
            //
            // Rule 516:  Interfacesopt ::= $Empty
            //
        case 516: {
            //#line 1059 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 517:  Interfacesopt ::= Interfaces
                //
        case 517:
            break;
            //
            // Rule 518:  Superopt ::= $Empty
            //
        case 518: {
            //#line 1062 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 519:  Superopt ::= Super
                //
        case 519:
            break;
            //
            // Rule 520:  TypeParametersopt ::= $Empty
            //
        case 520: {
            //#line 1065 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 521:  TypeParametersopt ::= TypeParameters
                //
        case 521:
            break;
            //
            // Rule 522:  ClassModifiersopt ::= $Empty
            //
        case 522: {
            //#line 1068 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 523:  ClassModifiersopt ::= ClassModifiers
                //
        case 523:
            break;
            //
            // Rule 524:  Annotationsopt ::= $Empty
            //
        case 524: {
            //#line 1071 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 525:  Annotationsopt ::= Annotations
                //
        case 525:
            break;
            //
            // Rule 526:  TypeDeclarationsopt ::= $Empty
            //
        case 526: {
            //#line 1074 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 527:  TypeDeclarationsopt ::= TypeDeclarations
                //
        case 527:
            break;
            //
            // Rule 528:  ImportDeclarationsopt ::= $Empty
            //
        case 528: {
            //#line 1077 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 529:  ImportDeclarationsopt ::= ImportDeclarations
                //
        case 529:
            break;
            //
            // Rule 530:  PackageDeclarationopt ::= $Empty
            //
        case 530: {
            //#line 1080 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 531:  PackageDeclarationopt ::= PackageDeclaration
                //
        case 531:
            break;
            //
            // Rule 532:  WildcardBoundsOpt ::= $Empty
            //
        case 532: {
            //#line 1083 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 533:  WildcardBoundsOpt ::= WildcardBounds
                //
        case 533:
            break;
            //
            // Rule 534:  AdditionalBoundListopt ::= $Empty
            //
        case 534: {
            //#line 1086 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 535:  AdditionalBoundListopt ::= AdditionalBoundList
                //
        case 535:
            break;
            //
            // Rule 536:  TypeBoundopt ::= $Empty
            //
        case 536: {
            //#line 1089 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 537:  TypeBoundopt ::= TypeBound
                //
        case 537:
            break;
            //
            // Rule 538:  TypeArgumentsopt ::= $Empty
            //
        case 538: {
            //#line 1092 "GJavaParser.g"
            setResult(nullptr);
            break;
        }
                //
                // Rule 539:  TypeArgumentsopt ::= TypeArguments
                //
        case 539:
            break;
            //#line 355 "btParserTemplateF.gi


        default:
            break;
        }
        return;
    }
};

