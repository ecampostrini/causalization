#include <occurrence_checker.h>

bool
Occurrence_checker::check_occurrence(VertexProperties *var, AST_Equation eq){
	variable = var;
	equation = eq;
	switch(eq->equationType()){
		case EQEQUALITY:
			AST_Equation_Equality eqEquality = eq->getAsEquality();
			return foldTraverse(eqEquality->left()) || foldTraverse(eqEquality->right());
		case EQFOR:
			AST_Equation_For eqFor = eq->getAsFor();
			/*for the moment we just handle FORS with 1 equationi
			 * and we suppose there are no nested loops */
			ASSERT(eqFor->equationList()->size() == 1);
			AST_Equation insideEq = eqFor->equationList()->front();
			switch(insideEq){
				case EQEQUALITY:		
					AST_Equation_Equality eqEquality = eq->getAsEquality();
					return foldTraverse(eqEquality->left()) || foldTraverse(eqEquality->right());
				case EQFOR:
					ERROR("Occurrence_checker::check_occurrence: Nested fors not supported yet\n");
				default:
					ERROR("Occurrence_checker::check_occurrence: Equation inside of FOR\n"
					      "not supported");

			}
		default:
			ERROR("Occurrence_checker::checl_occurrence: Equation type not suppoorted\n");
	}
	return false;
}

/*supongo que si llegue hasta aca la expression no 
* continene operaciones binarias ni tampoco menos unarios.
* Suponiendo esto paso a checkear los 4 tipos posibles de
* expressiones en esta etapa: COMPREFERENCE, DERIVATIVE,
* EXPREAL Y EXPINTEGER
*/	

bool
Occurrence_checker::foldTraverseElement(AST_Expression exp){
	switch(exp->expressionType()){
		case EXPCOMPREF:{
			AST_Expression_ComponentReference exp_cref = exp->getAsComponentReference();
			if(exp_cref->names()->size() > 1){
				ERROR("Occurrence_checker::foldTraverseElement:\n"
						"For the momento just variables of the form 'a' or a[index]"
			}
			if(equation->equationType() == EQEQUALITY){
				cout << exp_cref->print();				
			}
				
				

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
