#include <occurrence_checker.h>

bool
Occurrence_checker::check_occurrence(AST_String vName, AST_Expression exp){
	varName = vName;
	switch(epx->expressionType()){
		case EQEQUALITY:
			AST_Equation_Equality eqEquality = exp->getAsEquality();
			return foldTraverse(eqEquality->left()) || foldTraverse(eqEquality->right());
		default:
			ERROR("Occurrence_checker::checl_occurrence: Equation type not suppoorted");		
	}
	return false;
}

/*supongo que si llegue hasta aca la expression no 
* continene operaciones binarias ni tampoco menos unarios.
* Suponiendo esto paso a checkear los 4 tipos posibles de
* expressiones en esta etapa: COMPREFERENCE, DERIVATIVE,
* EXPREAL Y EXPINTEGER*/	
bool
Occurrence_checker::foldTraverseElement(AST_Expression exp){
	switch(exp->expressionType()){
		case EXPCOMPREF:{
			AST_Expression_ComponentReference exp_cref = exp->getAsComponentReference();

			

			/* */		
		}		
		case EXPDERIVATIVE:
		default:
			/*nada por ahora -> posiblemente: return false*/
			ASSERT(0);
	}
}

bool
Occurrence_checker::foldTraverseElementUMinus(AST_Expression exp){
	return foldTraverse(exp->getAsUMinus()->exp());
}

bool
Occurrence_checker::foldTraverseElement(bool b1, bool b2, BinOpType bo_type){
	return b1 || b2;		
}
