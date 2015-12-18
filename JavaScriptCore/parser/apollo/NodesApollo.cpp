/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Adobe Systems Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ADOBE SYSTEMS INCORPORATED "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ADOBE SYSTEMS INCORPORATED
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"
#include "Nodes.h"
#include "NodeConstructors.h"

#if ENABLE(CONTEXT_SAMPLING)
#include "SamplerApollo.h"
#endif

#if PLATFORM(APOLLO)

namespace JSC
{

ScopeNode::~ScopeNode()
{
    //TODO: apollo integrate -> fix sampler
#if  ENABLE(CONTEXT_SAMPLING)
    if (SamplerApollo* sampler = *SamplerApollo::enabledSamplerReference()) {
        sampler->unregisterNodeHelper(this);
    }
#endif
}

bool ElementNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalLiteral)
		return false;
	for (const ElementNode *n = this; n; n = n->m_next)
		if (!n->m_node->isSafeForEval(level))
			return false;
	return true;
}
	
bool PropertyNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalLiteral)
		return false;
	// reject getters and setters.
	if (level < EvalAny && m_type != Constant)
		return false;
	return m_assign->isSafeForEval(level);
}
	
bool PropertyListNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalLiteral)
		return false;
	for (const PropertyListNode *n = this; n; n = n->m_next) {
		if (!n->m_node->isSafeForEval(level))
			return false;
	}
	return true;
}
	
typedef Vector<StatementNode*> StatementVector;
static bool statementVectorIsSafeForEval(const StatementVector& children, EvalLevel const level)
{
	StatementVector::const_iterator const childrenEnd = children.end();
	StatementVector::const_iterator i = children.begin();
	bool isSafe = true;
	while((i != childrenEnd) && isSafe) {
		isSafe = (*i)->isSafeForEval(level);
		++i;
	}
	return isSafe;
}
	
bool BlockNode::isSafeForEval(EvalLevel level)
{
	// Important optimization so that the typical case doesn't have to traverse the parse tree.
	if (level == EvalAny)
		return true;
	if (level == EvalNone)
		return false;
		
	return statementVectorIsSafeForEval(m_statements->statements(), level);
}
    
bool ScopeNode::isSafeForEval(EvalLevel level)    
{
    // Important optimization so that the typical case doesn't have to traverse the parse tree.
    if (level == EvalAny)
        return true;
    if (level == EvalNone)
        return false;
        
    return statementVectorIsSafeForEval(data()->m_statements->statements(), level);    
}
    
	
bool IfNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalReadOnly)
		return false;
	if (!m_condition->isSafeForEval(level))
		return false;
	ASSERT(m_ifBlock);
	return m_ifBlock->isSafeForEval(level);
}
	
bool IfElseNode::isSafeForEval(EvalLevel level)
{
	if (!IfNode::isSafeForEval(level))
		return false;
	ASSERT(m_elseBlock);
	return m_elseBlock->isSafeForEval(level);
}
	
bool TryNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalReadOnly)
		return false;
	if (!m_tryBlock->isSafeForEval(level))
		return false;
	if (m_catchBlock && !m_catchBlock->isSafeForEval(level))
		return false;
	if (m_finallyBlock && !m_finallyBlock->isSafeForEval(level))
		return false;
	return true;
}
	
bool CaseClauseNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalReadOnly)
		return false;
	if (m_expr && !m_expr->isSafeForEval(level))
		return false;
	return statementVectorIsSafeForEval(m_statements->statements(), level);
}
	
bool ClauseListNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalReadOnly)
		return false;
	for (const ClauseListNode *n = this; n; n = n->m_next)
		if (!n->m_clause->isSafeForEval(level))
			return false;
	return true;
}
	
bool CaseBlockNode::isSafeForEval(EvalLevel level)
{
	if (level < EvalReadOnly)
		return false;
	if (!m_list1->isSafeForEval(level))
		return false;
	if (m_defaultClause && !m_defaultClause->isSafeForEval(level))
		return false;
	return m_list2->isSafeForEval(level);
}

bool  CommaNode::isSafeForEval(EvalLevel level) 
{ 
    if (level < EvalReadOnly)
        return false;
    
    ASSERT(m_expressions.size() > 1);
    for (size_t i = 0; i < m_expressions.size() - 1; i++)
        if (!m_expressions[i]->isSafeForEval(level))
            return false;
    
    return true; 
}

#if ENABLE(APOLLO_PROFILER)
void updateFuncExprNodeProfilerIdent(ExpressionNode* node, const ExpressionNode* base, const Identifier& ident)
{
	ASSERT(node);
	ASSERT(node->isFuncExprNode());
    
	FuncExprNode* funcExprNode = static_cast<FuncExprNode*>(node);
    if (base) {
        UString name(ident.ustring());
        const UString dotString(".");

        const ExpressionNode* baseIterator = base;
        while (baseIterator && baseIterator->isDotAccessorNode()) {
            const DotAccessorNode* const dotAccessorNode = static_cast<const DotAccessorNode*>(baseIterator);
            name = makeString(dotAccessorNode->identifier().ustring(), dotString, name);
            baseIterator = dotAccessorNode->base();
        }

        if (baseIterator && baseIterator->isResolveNode()) {
            const ResolveNode* const resolveNode = static_cast<const ResolveNode*>(baseIterator);
            name = makeString(resolveNode->identifier().ustring(), dotString, name);
        }

        funcExprNode->body()->setContextualName(name);
    } else {
        funcExprNode->body()->setContextualName(ident.ustring());
    }
}
#else
void updateFuncExprNodeProfilerIdent(ExpressionNode*, const ExpressionNode*, const Identifier&)
{
}
#endif


} /*namespace JSC*/

#endif /*PLATFORM(APOLLO)*/

