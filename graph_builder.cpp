#include <causalize/causalize2/graph_builder.h>
#include <boost/graph/adjacency_list.hpp>
#include <util/ast_util.h>
#include <mmo/mmo_class.h>

using namespace boost;
using namespace std;

ReducedGraphBuilder::ReducedGraphBuilder(MMO_Class mmo_cl):GraphBuilder(mmo_cl){
	symbolTable = mmo_cl->getVarSymbolTable();
}

AST_Real
ReducedGraphBuilder::eval(AST_Expression exp) {
	EvalExp evaluator(symbolTable);
  	AST_Expression result =  evaluator.eval(exp);
  	ERROR_UNLESS(result->expressionType() == EXPREAL || result->expressionType() == EXPINTEGER, "RangeIterator::getVal:\n"
      "Expression type should be EXPREAL or EXPINTEGER \n");
	if (result->expressionType() == EXPREAL) {
		return result->getAsReal()->val();
	} else {
		return result->getAsInteger()->val();
	}
}

/* @Return: size of the range of the FOR
 */

AST_Integer
ReducedGraphBuilder::getForRangeSize(MMO_Equation eq){
	/*casteamos a forEquation y checkeamos que sea un indice aceptable*/	
	AST_Integer size = 0;
	AST_Equation_For eqFor = eq->getAsFor();
	
	AST_ForIndexLis forIndexList =  forEq->forIndexList();
	ERROR_UNLESS(forIndexList->size() == 1, "graph_builder:\n For Loop with more
											than one index is not supported yet\n");
	AST_ForIndex forIndex = forIndexList->front();
	AST_Expression inExp = forIndex->in_exp();
	ExpressionType indexExpType = inExp->expressionType();

	if(indexExpType == EXPBRACE){
		AST_Expression_Brace braceExp = inExp->getAsBrace();
		size = braceExp->arguments()->size();		
	}else if(indexExpType == EXPRANGE){
		/*some definitions*/
		AST_Real first, temp;
		AST_Expression_Range rangeExp = inExp->getAsRange();
		AST_ExpressionList range = rangeExp->expressionList();
		AST_ExpressionListIterator it = range->begin();

		first = eval(current_element(it))
		it = range->next();
		temp = eval(current_element(++it));
		it++;
		if(it == range->end());
			/* here temp == the last element */
			while (temp > first){
				first++;
				size++;		
			}
		else{
			AST_Real step, last;
			step = temp;
			last = eval(current_element(it));		
			while( last > first){
				first += step;		
				size++;
			}
		}
	}else{ ERROR("Expression in FOR Index not supported\n");}
	return size;
}

CausalizationGraph 
ReducedGraphBuilder::makeGraph(){
	
	/* Create nodes for the Equations*/
	MMO_EquationListIterator it;
	foreach(it, mmo_class->getEquations()){
		VertexProperties *vp = new VertexProperties;
		vp->type = E;
		vp->equation = current_element(it);
		vp->eqType = current_element(it)->equationType();
		/*
		switch(vp->eqType){
			EQFOR:
				vp->count = getForRangeSize(vp->equation);
				break;
			default:
				vp->count = 1;
		}
		*/
		add_vertex(*vp, graph);
	}

	/* Create nodes for the unkowns: We iterate through the VarSymbolTable  */
	//VarSymbolTable symbolTable= mmo_class->getVarSymbolTable();	
	for(int i = 0; i < symbolTable->count(); i++){
		VarInfo	varInfo = symbolTable->varInfo(i);
		if( !varInfo->isConstant() && !varInfo->builtIn()
			&& !varInfo->isDiscrete() && !varInfo->isParameter()){
			
			//cout << varInfo->modification()->print();
		}
	}
	return graph;
	/* Create the edges */
}

