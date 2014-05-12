#include <causalize/causalize2/graph_builder.h>
#include <boost/graph/adjacency_list.hpp>
#include <util/ast_util.h>

using namespace boost;
using namespace std;

ReducedGraphBuilder::ReducedGraphBuilder(MMO_Class mmo_cl):GraphBuilder(mmo_cl){}

/* We get the expression of the For index, process it
 * and return the range of the variable. We don't check if
 * its a For equation since that was done before the calling
 */


int 
ReducedGraphBuilder::processForRange(MMO_Equation eq){
	/*casteamos a forEquation y checkeamos que sea un indice aceptable*/	
	AST_Equation_For eqFor = eq->getAsFor();
	
	AST_ForIndexLis forIndexList =  forEq->forIndexList();
	ERROR_UNLESS(forIndexList->size() == 1, "graph_builder:\n For Loop with more
											than one index is not supported yet\n");
	AST_ForIndex forIndex = forIndexList->front();
	AST_Expression inExp = forIndex->in_exp();
	ExpressionType indexExpType = inExp->expressionType();

	switch(indexExpType){
		EXPBRACE:
									
			break;
		EXPRANGE:
			break;
		default:
			ERROR("Expression in For Index not supported\n");
	}
		
	/*sacamos el forindex*/
	/*ast_expression final - ast_expression inicial */
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
				vp->count = processForRange(vp->equation);
				break;
			default:
				vp->count = 1;
		}
		*/
		add_vertex(*vp, graph);
	}

	/* Create nodes for the unkowns: We iterate through the VarSymbolTable  */
	VarSymbolTable symbolTable= mmo_class->getVarSymbolTable();	
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

