#include <causalize/causalize2/graph_builder.h>
#include <causalize/causalize2/occurrence_checker.h>
#include <causalize/causalize2/graph_printer.h>
#include <causalize/causalize2/graph_definition.h>


#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>
#include <util/ast_util.h>
#include <mmo/mmo_class.h>

#ifdef ENABLE_DEBUG_MSG
#define DEBUG_MSG(str) do {std::cout << str << std::endl;} while( false )
#else
#define DEBUG_MSG(str) do {} while( false )
#endif

using namespace std;
using namespace boost;
using namespace boost::icl;

ReducedGraphBuilder::ReducedGraphBuilder(MMO_Class mmo_cl):GraphBuilder(mmo_cl){
	symbolTable = mmo_cl->getVarSymbolTable();
	state_finder = new StateVariablesFinder(mmo_cl);
}

ReducedGraphBuilder::~ReducedGraphBuilder(){
	delete equationDescriptorList;
	delete unknownDescriptorList;
}

AST_Integer
ReducedGraphBuilder::getDimension(AST_Expression modification){
	EvalExp evaluator(symbolTable);
	AST_Expression result = evaluator.eval(modification);
	ERROR_UNLESS(result->expressionType() == EXPINTEGER, "ReducedGraphBuilder::getDimension\n"
				 "Array dimension must be an Integer\n");
	return result->getAsInteger()->val();
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
	AST_Integer size = 1;
	AST_Equation_For forEq = eq->getAsFor();
	AST_ForIndexList forIndexList =  forEq->forIndexList();
	ERROR_UNLESS(forIndexList->size() == 1, "graph_builder:\n For Loop with more"
											"than one index is not supported yet\n");
	AST_ForIndex forIndex = forIndexList->front();
	AST_Expression inExp = forIndex->in_exp();
	ExpressionType indexExpType = inExp->expressionType();

	if(indexExpType == EXPBRACE){
		AST_Expression_Brace braceExp = inExp->getAsBrace();
		size = braceExp->arguments()->size();		
	}else if(indexExpType == EXPRANGE){
		/*some more definitions*/
		AST_Real first, temp;
		AST_Expression_Range rangeExp = inExp->getAsRange();
		AST_ExpressionList range = rangeExp->expressionList();
		AST_ExpressionListIterator it = range->begin();

		first = eval(current_element(it));
		it++;
		temp = eval(current_element(it));
		it++;
		if(it == range->end()){
			/* here the step is 1 and temp == the last element */
			while (temp > first){
				first += 1.;
				size++;	
			}
		}
		else{
			ERROR("ReducedGraphBuilder::getForRangeSize: FOR ranges with leaps not supported yet\n");
			/* here temp == step */
			AST_Real last;
			last = eval(current_element(it));		
			while( last > first){
				first += temp;		
				size++;
			}
		}
	}else{ ERROR("Expression in FOR Index not supported\n");}

	return size;
}


CausalizationGraph 
ReducedGraphBuilder::makeGraph(){
	equationDescriptorList = new list<Vertex>();
	unknownDescriptorList = new list<Vertex>();
	/* Create nodes for the Equations*/
	MMO_EquationListIterator it;
	foreach(it, mmo_class->getEquations()){
		static AST_Integer index = 0;
		VertexProperties *vp = new VertexProperties;
		vp->type = E;
		vp->equation = current_element(it);
		vp->eqType = current_element(it)->equationType();
		//DEBUG('g', "Equation: %s, eqType: %d\n", vp->equation->print().c_str(), vp->eqType);
		switch(vp->eqType){
			case EQFOR:
				vp->count = getForRangeSize(vp->equation);
				//DEBUG('g', "ForRange: %d\n", vp->count);
				break;
			default:
				vp->count = 1;
		}
		vp->index = index++;
		vp->degree = 0;
		Vertex eqDescriptor = add_vertex(*vp, graph);
		equationDescriptorList->push_back(eqDescriptor);
	}

	/* Create nodes for the unkowns: We iterate through the VarSymbolTable 
	 * and create one vertex per unknown */
	state_finder->findStateVariables();
	for(int i = 0; i < symbolTable->count(); i++){
		static AST_Integer index = 0;
		VarInfo	varInfo = symbolTable->varInfo(i);
		if( !varInfo->isConstant() && !varInfo->builtIn()
			&& !varInfo->isDiscrete() && !varInfo->isParameter()){

			Type varType = varInfo->type();
			VertexProperties *vp = new VertexProperties;			
			vp->type = U;
			vp->variableName = symbolTable->key(i);
			if(varType->getType() == TYREAL){
				if(varInfo->isState()){
					vp->isState = true;
				}else{
					vp->isState = false;
				}
				vp->count = 0;
				
			}
			else if(varType->getType() == TYARRAY){
				Type_Array array_type = varType->getAsArray();
				if(array_type->arrayOf()->getType() == TYARRAY){
					ERROR("ReducedGraphBuilder::makeGraph Arrays of arrays are not supported yet\n");			
				}
				if(varInfo->isState()){
					// que hago acaaa!???		
					vp->isState = true;
				}else{
					vp->isState = false;		
				}
				vp->count = getDimension(array_type->dimension());
			}
			else{ERROR("ReducedGraphBuilder::makeGraph A variable shouldn't have the type %s at this point. Compiler's mistake.\n", varType->print().c_str());}
			vp->index = index++;
			vp->degree = 0;
			Vertex unknownDescriptor = add_vertex(*vp, graph);
			unknownDescriptorList->push_back(unknownDescriptor);
		}
	}


	//Create the edges 
	list<Vertex>::iterator eqsIt, unIt;
	
	#ifdef ENABLE_DEBUG_MSG
	if(debugIsEnabled('g')){
		DEBUG_MSG("Ecuaciones");
		foreach(eqsIt, equationDescriptorList){
			DEBUG_MSG(graph[current_element(eqsIt)].index << ": " << graph[current_element(eqsIt)].equation->print()) ;
		}
		DEBUG_MSG("Incognitas");
		foreach(unIt, unknownDescriptorList){
			string var_name;
			if(graph[current_element(unIt)].isState){
				var_name = "der(" + graph[current_element(unIt)].variableName + ")";
			}else{
				var_name = graph[current_element(unIt)].variableName;		
			}
			DEBUG_MSG(graph[current_element(unIt)].index << ": " << var_name) ;
		}
	}
	#endif
	
	Occurrence_checker *oc = new Occurrence_checker(symbolTable);
	//DEBUG('g', "Adjacency list as (equation_index, unknown_index, number_of_edges) {genericIndex of each edge}:\n");
	foreach(eqsIt, equationDescriptorList){
		foreach(unIt,unknownDescriptorList){
			if(oc->check_occurrence(graph[current_element(unIt)], graph[current_element(eqsIt)].equation)){
				list<EdgeProperties> edgeList = oc->getOccurrenceIndexes();
				graph[current_element(unIt)].degree += edgeList.size();
				graph[current_element(eqsIt)].degree += edgeList.size();
				for(list<EdgeProperties>::iterator edgeIt = edgeList.begin(); edgeIt != edgeList.end(); edgeIt++){
					Edge descriptor;
					bool result;
					tie(descriptor, result) = add_edge(current_element(eqsIt),current_element(unIt),current_element(edgeIt), graph);
					if(!result){
						ERROR("makeGraph: Error while adding the edges to the graph\n");
					}
				}
				//DEBUG('g', "(%d, %d, %d), ", graph[current_element(eqsIt)].index ,graph[current_element(unIt)].index,edgeList.size());
				/*#ifdef ENABLE_DEBUG_MSG
				graph_traits<CausalizationGraph>::out_edge_iterator begin, end;			
				stringstream stri;
				tie(begin, end) = out_edges(current_element(unIt), graph);
				stri << "{";
				while(begin!=end){
					Vertex eq = target(*begin, graph);
					if(eq != current_element(eqsIt)){
						begin++;
						continue;
					}
					if( graph[current_element(begin)].genericIndex.first ){
						if(!stri.str().empty()) stri << ", ";
						stri << graph[current_element(begin)].genericIndex.first << " * i + " << graph[current_element(begin)].genericIndex.second;
						interval_set<int>::iterator it1, it2;
						it1 = graph[current_element(begin)].indexInterval.begin();
						it2 = graph[current_element(begin)].indexInterval.end();
						stri << ", " << first(*it1) << " : " << last(*--it2);
					}
					begin++;
				}
				stri << "}";
				cout << stri.str() << endl;
				#endif
				*/
			}
		}		
	}
	DEBUG('g', "\n");
	return graph;
}

