#include <causalize/causalize2/occurrence_checker.h>

#include <iostream>
#include <assert.h>
using namespace std;


Occurrence_checker::Occurrence_checker(VarSymbolTable sTable){
	equalExp = new EqualExp(sTable);
	evaluator = new EvalExp(sTable);
	occurrenceSetList = new list<EdgeProperties*>;
	symbolTable = sTable;
}

list<EdgeProperties*>*
Occurrence_checker::getOccurrenceIndexes(){
	return occurrenceSetList;		
}

bool
Occurrence_checker::check_occurrence(VertexProperties var, AST_Equation eq){
	variable = var;
	equation = eq;
	occurrenceSetList->clear();
	switch(eq->equationType()){
		case EQEQUALITY:{
			AST_Equation_Equality eqEquality = eq->getAsEquality();
			bool left = foldTraverse(eqEquality->left());
			bool right = foldTraverse(eqEquality->right());
			return left || right;
			break;
		}
		case EQFOR:{
			AST_Equation_For eqFor = eq->getAsFor();
			/*for the moment we just handle FORS with 1 equation
			 * and we suppose there are no nested loops */
			assert(eqFor->equationList()->size() == 1);
			AST_Equation insideEq = eqFor->equationList()->front();
			switch(insideEq->equationType()){
				case EQEQUALITY:{		
					AST_Equation_Equality eqEquality = insideEq->getAsEquality();
					bool left = foldTraverse(eqEquality->left());
					bool right = foldTraverse(eqEquality->right());
					return left || right;
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

ForIndexIterator*
Occurrence_checker::processInExp(AST_Expression inExp, VarSymbolTable symbolTable){
	ForIndexIterator *iterator;
	switch(inExp->expressionType()){
		case EXPRANGE:
			iterator = new RangeIterator(inExp->getAsRange(), symbolTable);
				return iterator;
				break;
		case EXPBRACE:
			iterator = new BraceIterator(inExp->getAsBrace());
			return iterator;
			break;
			default:
				ERROR("process_for_equations:\n"
					"Equation type not supported in forIndex inExp\n");
	}
	return NULL;
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
Occurrence_checker::arrayOccurrence(AST_Expression_ComponentReference cref_exp){
	EdgeProperties *newEdge = new EdgeProperties;	
	switch(equation->equationType()){
		case EQEQUALITY:{
			newEdge->genericIndex = NULL;
			AST_Integer indexVal = evalIndexExpression(cref_exp->indexes()->front()->front());
			//DEBUG('c', "Index inserted: %d\n", indexVal);
			newEdge->indexes.insert(indexVal);
			break;
		}
		case EQFOR:{
			newEdge->genericIndex = cref_exp->indexes()->front()->front();
			AST_Equation_For eqFor = equation->getAsFor();
			ERROR_UNLESS(eqFor->forIndexList()->size() == 1,
				"Occurrence_checker::arrayOccurrence:\n"
				"forIndexList with more than 1 forIndex are not supported yet\n");
			ForIndexIterator *forIndexIter = processInExp(eqFor->forIndexList()->front()->in_exp(), symbolTable);
			AST_String indexName = eqFor->forIndexList()->front()->variable();
			AST_Expression_ComponentReference compRef = (AST_Expression_ComponentReference) newAST_Expression_ComponentReferenceExp(indexName);
			while(forIndexIter->hasNext()){
				AST_Real indexVal = forIndexIter->next();
				AST_Expression_Integer indexExp = (AST_Expression_Integer) newAST_Expression_Integer(static_cast<AST_Integer>(indexVal));
				AST_Expression result = evaluator->eval(compRef, indexExp, cref_exp->indexes()->front()->front());
				switch(result->expressionType()){
					case EXPINTEGER:{
						AST_Integer val = result->getAsInteger()->val(); 
						newEdge->indexes.insert(val);
						//DEBUG('c', "Index inserted: %d\n",  val);
						break;
					}
					default:
						assert(0);
				}
			}
			break;
		}
		default:		
			ERROR("Occurrence_checker::foldTraverseElement: equation not supported, compiler's mistake\n");
		}
		occurrenceSetList->push_back(newEdge);
}

/* If we got to this point then the expression doesn't 
 * contain binops nor unary minuses. Assuming this, we
 * just check for the 4 possible expressions we can have:
 * EXPCOMPREF, EXPDERIVATIVE, EXPREAL and EXPINTEGER
*/
bool
Occurrence_checker::foldTraverseElement(AST_Expression exp){
	switch(exp->expressionType()){
		case EXPCOMPREF:{
			AST_Expression_ComponentReference exp_cref = exp->getAsComponentReference();
			if(exp_cref->names()->front()->compare(variable.variableName)){break;}
			if(exp_cref->indexes()->size() > 1 || exp_cref->indexes()->front()->size() > 1){
				ERROR("Occurrence_checker::foldTraverseElement:\n"
						"Expression : %s\n"
						"For the momento just variables of the form 'a' or a[index]\n", 
						exp_cref->print().c_str());
			}
			if(!exp_cref->indexes()->front()->empty()){
				/*if its an array*/
				arrayOccurrence(exp_cref);
			}else{
				/*if its a simple var we don't care in what kind of equation it
				 * appears */
				EdgeProperties *newEdge = new EdgeProperties;
				newEdge->genericIndex = NULL;
				occurrenceSetList->push_back(newEdge);
			}

			return true;
			break;
		}		
		case EXPDERIVATIVE:
			break;
		default:
			/*nothing for the moment -> possibly: return false*/
			//cout << exp->print() << ", " << exp->expressionType() << endl;
			assert(exp->expressionType() == EXPREAL || exp->expressionType() == EXPINTEGER);
			return false;
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
