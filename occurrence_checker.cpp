#include <causalize/causalize2/occurrence_checker.h>

#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>
#include <iostream>
#include <assert.h>
using namespace std;
using namespace boost;
using namespace boost::icl;

Occurrence_checker::Occurrence_checker(VarSymbolTable sTable){
	evaluator = new EvalExp(sTable);
	symbolTable = sTable;
}

list<EdgeProperties>
Occurrence_checker::getOccurrenceIndexes(){
	return edgeList;
}

bool
Occurrence_checker::check_occurrence(VertexProperties var, AST_Equation eq){
	variable = var;
	equation = eq;
	bool result = false;
	switch(eq->equationType()){
		case EQEQUALITY:{
			edgeList.clear();
			genericIndexSet.clear();
			simpleIndex.clear();
			all_indexes = false;
			AST_Equation_Equality eqEquality = eq->getAsEquality();
			bool left = foldTraverse(eqEquality->left());
			bool right = foldTraverse(eqEquality->right());
			result = left || right;
			if(result && variable.count == 0){
				//its just a simple var (no array)
				EdgeProperties newEdge;
				newEdge.genericIndex.first = newEdge.genericIndex.second = 0;
				newEdge.indexInterval.add(discrete_interval<int>::open(0, 0));
				edgeList.push_back(newEdge);
			}else{
				//its an array
				if(!genericIndexSet.empty()){
					//occurrence inside a FOR
					for(set< pair<AST_Integer, AST_Integer> >::iterator it = genericIndexSet.begin(); it != genericIndexSet.end(); it++){
						EdgeProperties newEdge;
						newEdge.genericIndex = *it;
						newEdge.indexInterval.add(forIndexInterval);
						edgeList.push_back(newEdge);
					}
				}
				if(!simpleIndex.empty()){
					//occurrence of a particular position of the array
					for(set<AST_Integer>::iterator it = simpleIndex.begin(); it != simpleIndex.end(); it++){
						EdgeProperties newEdge;
						newEdge.genericIndex.first = 1;
						newEdge.genericIndex.second = 0;
						newEdge.indexInterval.add(discrete_interval<int>::closed(*it, *it));
						edgeList.push_back(newEdge);
					}
				}
				if(all_indexes){
					EdgeProperties newEdge;
					newEdge.genericIndex.first = 1;
					newEdge.genericIndex.second = 0;
					newEdge.indexInterval.add(discrete_interval<int>::closed(1, variable.count));
					edgeList.push_back(newEdge);
				}
			}
			return result;
			break;
		}
		case EQFOR:{
			//for the time being we just handle FORS with 1 equation
			//and we suppose there are no nested loops
			AST_Equation_For eqFor = eq->getAsFor();
			AST_Equation insideEq = eqFor->equationList()->front();
			ERROR_UNLESS(eqFor->forIndexList()->size() == 1,
				"Occurrence_checker::check_occurrence: "
				"forIndexList with more than 1 forIndex are not supported yet\n");
			ERROR_UNLESS(eqFor->equationList()->size() == 1,
				 "Occurrence_checker::check_occurrence: "
				 "More than 1 equation inside a FOR is not supported yet\n");
			ERROR_UNLESS(insideEq->equationType() == EQEQUALITY, "Occurrence_checker::check_occurrence: "
				 "Only single equations are allowed inside a FOR\n");
			//we calculate the ranges
			pair<AST_Integer,AST_Integer> ranges = get_for_range(eqFor->forIndexList()->front()->in_exp(), symbolTable);
			forIndexInterval = discrete_interval<int>::closed(ranges.first, ranges.second);
			result = check_occurrence(var, insideEq);
			return result;
			break;
		}
		default:
			ERROR("Occurrence_checker::checl_occurrence: Equation type not suppoorted\n");
	}
	return result;
}

pair<AST_Integer, AST_Integer>
Occurrence_checker::get_for_range(AST_Expression inExp, VarSymbolTable symbolTable){
	pair <AST_Integer, AST_Integer> range;
	switch(inExp->expressionType()){
		case EXPRANGE:{
			RangeIterator iterator(inExp->getAsRange(), symbolTable);
			range.first = (AST_Integer) iterator.begin();
			range.second = (AST_Integer) iterator.end();
			return range;
			break;
		}
		case EXPBRACE:
			ERROR("Occurrence_checker::proecessInExp:\n"
			      "Braces in FOR expressions are not supported yet\n");
		default:
				ERROR("process_for_equations:\n"
					"Equation type not supported in forIndex inExp\n");
	}
	return range;
}

/*
* Here we expect an expression like: a*i + b, where 'i'
* is the for variable.
*/
void
Occurrence_checker::add_generic_index(AST_Expression index){
	AST_Integer a, b, mult;
	if(index->expressionType() == EXPBINOP){
		AST_Expression_BinOp binop = index->getAsBinOp();	
		if(binop->binopType() == BINOPADD || binop->binopType() == BINOPSUB){
			//the expression is a*i + b
			mult = (binop->binopType() == BINOPADD) ? 1 : (-1);
			b = binop->right()->getAsInteger()->val();		
			b *= mult;
			if(binop->left()->expressionType() == EXPBINOP && binop->left()->getAsBinOp()->binopType() == BINOPMULT){
				AST_Expression_BinOp binopMult = binop->left()->getAsBinOp();
				a = binopMult->left()->getAsInteger()->val();
			}else if(binop->left()->expressionType() == EXPINTEGER || binop->left()->expressionType() == EXPREAL){
				AST_Expression_Integer _a = binop->left()->getAsInteger();
				a = _a->val();
			}else if(binop->left()->expressionType() == EXPCOMPREF){
				a = 1;		
			}
			else{
				ERROR("Occurrence_checker::add_generic_index, error in expression %s. Index of arrays"
		      		  " must have the form: a*i + b for the moment.\n", index->print().c_str());
			}
		}else if(binop->binopType() == BINOPMULT){
			//the expression is a*i
			b = 0;		
			a = binop->left()->getAsInteger()->val();
		}
		genericIndexSet.insert(pair<AST_Integer, AST_Integer> (a,b));
	}
	else if(index->expressionType() == EXPCOMPREF){
		//the expression is just 'i'
		a = 1;
		b = 0;
		genericIndexSet.insert(make_pair(a,b));
			
	}
	else{
		ERROR("Occurrence_checker::add_generic_index: index of arrays"
		      " must have the form: a*i + b for the momento\n");
	}
}

void
Occurrence_checker::arrayOccurrence(AST_Expression_ComponentReference cref_exp){
	AST_Expression indexExp = cref_exp->indexes()->front()->front();
	if(indexExp == NULL){
		//we use the array in all of its indexes
		all_indexes = true;
	}else{
		AST_Expression indexResult = evaluator->eval(indexExp);
		if(indexResult->expressionType() == EXPINTEGER || indexResult->expressionType() == EXPREAL){
			simpleIndex.insert(indexResult->getAsInteger()->val());
		}else if(indexResult->expressionType() == indexExp->expressionType()){
			add_generic_index(indexExp);		
		}else{
			ERROR("Occurrence_checker::arrayOccurrence: wrong value returned by "
		    	  "the evaluator\n");		
		}
	}
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
			if(variable.isState){
				return false;
			}
			if(variable.count != 0){
				//if its an array
				arrayOccurrence(exp_cref);
			}else{
				//if its a simple var we don't care in what kind of equation it
				// appears
			}
			return true;
			break;
		}
		case EXPDERIVATIVE:{
			//if(variable.isState){
				//bool result = false;
				AST_Expression_Derivative exp_der = exp->getAsDerivative();
				ERROR_UNLESS(exp_der->arguments()->size() == 1, "Derivatives can have only one argument\n");
				ERROR_UNLESS(exp_der->arguments()->front()->expressionType() == EXPCOMPREF, "Only simple variable derivatives are allowed for the moment\n");
				AST_Expression_ComponentReference exp_cref = exp_der->arguments()->front()->getAsComponentReference();
				if(exp_cref->names()->front()->compare(variable.variableName)){break;}
				if(exp_cref->indexes()->size() > 1 || exp_cref->indexes()->front()->size() > 1){
					ERROR("Occurrence_checker::foldTraverseElement:\n"
							"Expression : %s\n"
							"For the momento just variables of the form 'a' or a[index]\n", 
							exp_cref->print().c_str());
				}
				if(variable.count != 0){
					//if its an array
					arrayOccurrence(exp_cref);
				}else{
					//if its a simple var we don't care in what kind of equation it
					// appears
				}
				//result = foldTraverseElement(exp_der->arguments()->front());
				return true;
			//}
			break;
		}
		case EXPCALL:{
			//we check the arguments
			bool result = false;
			AST_Expression_Call exp_call = exp->getAsCall();
			AST_ExpressionListIterator it;
			foreach(it, exp_call->arguments()){
				result = foldTraverse(current_element(it));
				if(result){
					break;
				}
			}
			return result;
			break;
		}	
		default:
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
