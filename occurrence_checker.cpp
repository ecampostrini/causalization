#include <causalize/causalize2/occurrence_checker.h>

#include <boost/icl/discrete_interval.hpp>
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
	bool result;
	occurrenceSetList->clear();
	switch(eq->equationType()){
		case EQEQUALITY:{
			AST_Equation_Equality eqEquality = eq->getAsEquality();
			bool left = foldTraverse(eqEquality->left());
			bool right = foldTraverse(eqEquality->right());
			//return left || right;
			result = left || right;
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
					//return left || right;
					result = left || right;
					break;
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
	//return false;
	if(!genericIndexSet.empty()){
		for(set< pair<AST_Integer, AST_Integer> >::iterator it = genericIndexSet.begin(); it != genericIndexSet.end(); it++){
			EdgeProperties *newEdge = new EdgeProperties;			
			newEdge->genericIndex = *it;
			newEdge->indexRange = indexes;
			newEdge->simpleIndex = -1;
			occurrenceSetList->push_back(newEdge);
		}
	}
	if(!simpleIndex.empty()){
		for(set<AST_Integer>::iterator it = simpleIndex.begin(); it != simpleIndex.end(); it++){
			EdgeProperties *newEdge = new EdgeProperties;
			newEdge->simpleIndex = -1;
			occurrenceSetList->push_back(newEdge);
		}
	}
	if(simpleIndex.empty() && genericIndexSet.empty() && result){
		//its just a simple var
		EdgeProperties *newEdge = new EdgeProperties;		
		occurrenceSetList->push_back(newEdge);
		newEdge->simpleIndex = -2;

	}
	return result;
}

pair<AST_Integer, AST_Integer>
Occurrence_checker::get_for_range(AST_Expression inExp, VarSymbolTable symbolTable){
	pair <AST_Integer, AST_Integer> range;
	switch(inExp->expressionType()){
		case EXPRANGE:
			RangeIterator *iterator;
			iterator = new RangeIterator(inExp->getAsRange(), symbolTable);
			range.first = (AST_Integer) iterator->begin();
			range.second = (AST_Integer) iterator->end();
			delete iterator;
			return range;
			break;
		case EXPBRACE:
			ERROR("Occurrence_checker::proecessInExp:\n"
			      "Braces in FOR expressions are not supported yet\n");
			//iterator = new BraceIterator(inExp->getAsBrace());
			/*return iterator;
			break;*/
		
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
			}else{
								
				ERROR("Occurrence_checker::add_generic_index: index of arrays"
		      		  "must have the form: a*i + b for the moment\n");
			}
		}else if(binop->binopType() == BINOPMULT){
			b = 0;		
			a = binop->left()->getAsInteger()->val();
		}
		genericIndexSet.insert(pair<AST_Integer, AST_Integer> (a,b));

	}else{
		ERROR("Occurrence_checker::add_generic_index: index of arrays"
		      "must have the form: a*i + b for the moment\n");
	}
}

void
Occurrence_checker::arrayOccurrence(AST_Expression_ComponentReference cref_exp){
	switch(equation->equationType()){
		case EQEQUALITY:{
			AST_Expression indexResult = evaluator->eval(cref_exp->indexes()->front()->front());
			switch(indexResult->expressionType()){
				case EXPINTEGER:
				case EXPREAL:
					simpleIndex.insert(indexResult->getAsInteger()->val());
					break;
				default:
					ERROR("Occurrence_checker::arrayOccurrence: wrong value returned by "
					      "the evaluator\n");		
			}
			break;
		}
		case EQFOR:{
			add_generic_index(cref_exp->indexes()->front()->front());
			AST_Equation_For eqFor = equation->getAsFor();
			ERROR_UNLESS(eqFor->forIndexList()->size() == 1,
				"Occurrence_checker::arrayOccurrence:\n"
				"forIndexList with more than 1 forIndex are not supported yet\n");
			pair<AST_Integer,AST_Integer> ranges = get_for_range(eqFor->forIndexList()->front()->in_exp(), symbolTable);
			indexes = boost::icl::construct< boost::icl::discrete_interval<int> > (ranges.first, ranges.second, boost::icl::interval_bounds::closed());	
			break;
		}
		default:		
			ERROR("Occurrence_checker::foldTraverseElement: equation not supported, compiler's mistake\n");
		}
		//occurrenceSetList->push_back(newEdge);
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
				/*EdgeProperties *newEdge = new EdgeProperties;
				newEdge->genericIndex = NULL;
				occurrenceSetList->push_back(newEdge);
				*/
			}

			return true;
			break;
		}
		case EXPDERIVATIVE:
			break;
		case EXPCALL:
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
