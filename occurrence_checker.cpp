#include <causalize/causalize2/occurrence_checker.h>
#include <iostream>
#include <assert.h>
using namespace std;


Occurrence_checker::Occurrence_checker(VarSymbolTable sTable){
	equalExp = new EqualExp(sTable);
	evaluator = new EvalExp(sTable);
	occurrenceSetList = new list<EdgeProperties*>;
}

bool
Occurrence_checker::check_occurrence(VertexProperties var, AST_Equation eq){
	variable = var;
	equation = eq;
	switch(eq->equationType()){
		case EQEQUALITY:{
			AST_Equation_Equality eqEquality = eq->getAsEquality();
			return foldTraverse(eqEquality->left()) || foldTraverse(eqEquality->right());
			break;
		}
		case EQFOR:{
			AST_Equation_For eqFor = eq->getAsFor();
			/*for the moment we just handle FORS with 1 equationi
			 * and we suppose there are no nested loops */
			assert(eqFor->equationList()->size() == 1);
			AST_Equation insideEq = eqFor->equationList()->front();
			switch(insideEq->equationType()){
				case EQEQUALITY:{		
					AST_Equation_Equality eqEquality = insideEq->getAsEquality();
					return foldTraverse(eqEquality->left()) || foldTraverse(eqEquality->right());
				}
				case EQFOR:
					ERROR("Occurrence_checker::check_occurrence: Nested fors not supported yet\n");
				default:
					ERROR("Occurrence_checker::check_occurrence: Equation inside of FOR\n"
					      "not supported");

			}
			break;
		}
		default:
			ERROR("Occurrence_checker::checl_occurrence: Equation type not suppoorted\n");
	}
	return false;
}

AST_Integer 
Occurrence_checker::evalIndexExpression(AST_Expression exp){
	AST_Integer returnValue;
	AST_Expression indexVal = evaluator->eval(exp);

	switch(indexVal->expressionType()){
		case EXPINTEGER:
		case EXPREAL:
			returnValue = indexVal->getAsInteger()->val();
			break;
		default:
			ERROR("Occurrence_checker::evalIndexExpression: wrong value returned by the "
					"evaluator\n");
	}

	return returnValue;
}

void
Occurrence_checker::arrayOccurrence(AST_Expression innerExp){
		
	switch(equation->equationType()){
		case EQEQUALITY:{
			EdgeProperties *newEdge = new EdgeProperties;	
			newEdge->genericIndex = NULL;
			//AST_Expression innerExp = exp_cref->indexes()->front()->front();
			AST_Integer indexVal = evalIndexExpression(innerExp);
			DEBUG('c', "index inserted: %d\n", indexVal);
			newEdge->indexes.insert(indexVal);
			occurrenceSetList->push_back(newEdge);
			break;
		}
		case EQFOR:{
			break;
		}
		default:		
			ERROR("Occurrence_checker::foldTraverseElement: equation not supported, compiler's mistake\n");
		}
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
			if(exp_cref->indexes()->size() > 1 || exp_cref->indexes()->front()->size() > 1){
				ERROR("Occurrence_checker::foldTraverseElement:\n"
						"Expression : %s\n"
						"For the momento just variables of the form 'a' or a[index]\n", 
						exp_cref->print().c_str());
			}
			if(!exp_cref->indexes()->front()->empty()){
				/*if its an array*/
				arrayOccurrence(exp_cref->indexes()->front()->front());
			}
			return true;
			break;
		}		
		case EXPDERIVATIVE:
			break;
		default:
			/*nada por ahora -> posiblemente: return false*/
			assert(exp->expressionType() == EXPREAL || exp->expressionType() == EXPINTEGER);
	}
	return false;
}

bool
Occurrence_checker::foldTraverseElementUMinus(AST_Expression exp){
	return foldTraverse(exp->getAsUMinus()->exp());
}

bool
Occurrence_checker::foldTraverseElement(bool b1, bool b2, BinOpType bo_type){
	return b1 || b2;		
}
