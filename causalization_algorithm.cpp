#include <causalize/causalize2/causalization_algorithm.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost;

CausalizationStrategy::CausalizationStrategy(CausalizationGraph g){
	graph = g;
	equations1toN = (MMO_EquationList) newMMO_EquationList;
	equationsNto1 = (MMO_EquationList) newMMO_EquationList;
	equationDescriptors = new list<Vertex>();
	unknownDescriptors = new list<Vertex>();	
	CausalizationGraph::vertex_iterator vi, vi_end;
	equationNumber = unknownNumber = 0;
	for(boost::tie(vi, vi_end) = vertices(graph); vi != vi_end; vi++){
		Vertex current_element = *vi;
		if(graph[current_element].type == E){
			equationNumber += graph[current_element].count;
			equationDescriptors->push_back(current_element);
		}
		else{
			unknownNumber += graph[current_element].count;
			unknownDescriptors->push_back(current_element);
		}
	}
	DEBUG('c', "Number of equations %d\n"
	           "Number of unknowns %d\n", 
			   equationNumber, unknownNumber);
	
	if(equationNumber != unknownNumber){
		ERROR("The model being causalized is not balanced.\n"
			  "There are %d equations and %d variables\n", 
			  equationNumber, unknownNumber);		
	}
}

void
CausalizationStrategy::remove_edge_from_array(Edge targetEdge, map<Edge, Vertex> toRemove){
		
}
void
CausalizationStrategy::remove_edge_from_array(Vertex targetVertex, Edge targetEdge){
	
	
}

void
CausalizationStrategy::causalize(){	
	list<Vertex>::iterator iter;
	foreach(iter, equationDescriptors){
		Vertex eq = current_element(iter);
		EquationType eqType = graph[current_element(iter)].eqType;
		if(eqType == EQEQUALITY){
			/*here we use the classic version of the algorithm*/
			if(out_degree(eq, graph) == 1){
				Edge e = *out_edges(eq, graph).first;			
				Vertex unknown = target(e,graph);
				if (graph[e].indexes.empty()){
					/*its a regular variable*/		
					remove_out_edge_if(unknown, boost::lambda::_1 != e, graph);
					/*TODO MAKECAUSAL*/
					equationNumber--;
					unknownNumber--;
					equationDescriptors->erase(iter);
					unknownDescriptors->remove(unknown);
				}else{
					/*its an array*/		
					assert(graph[e].indexes.size() == 1);
					/*TODO MAKECAUSAL*/
					/*TODO remove_index_from_array*/
					remove_edge_from_array(unknown, e);
					equationNumber--;
					if(--unknownNumber == 0){
						unknownDescriptors->remove(unknown);
					}
					equationDescriptors->erase(iter);
				}
			}else if(out_degree(eq, graph) == 0){
				ERROR("Problem is singular, not supported yet\n");
			}
		}else if(eqType == EQFOR){
			if(out_degree(eq,graph) == 1){
				Edge e = *out_edges(eq, graph).first;
				Vertex unknown = target(e, graph);
				if(graph[e].indexes.empty()){
					//its a regular variable and we just solve 
					//arrays in the FOR
				}else{
					/*only one variable in the FOR, we causalize it*/
					/*TODO MAKECAUSAL*/
					remove_edge_from_array(unknown, e);
					equationNumber -= graph[eq].count;
					unknownNumber -= graph[eq].count; //the array may have more variables 
					if(unknownNumber == 0){
						unknownDescriptors->remove(unknown);
					}
					equationDescriptors->erase(iter);
				}
			}else{
				/*if only one of the edges has weight == size of range
				 * then thats the one we are causalizing */
				CausalizationGraph::out_edge_iterator begin, end, it;
				Edge targetEdge; 
				//Vertex causalizedUnknown;
				AST_Integer sameWeight = 0;
				map<Edge, Vertex> toRemove;
				for(boost::tie(begin, end) = out_edges(eq, graph), it = begin; it != end; it++){
					Edge e = *it;
					Vertex unknown = target(e, graph);
					toRemove.insert(pair<Edge, Vertex>(e, unknown));
					if(graph[e].indexes.size() == (unsigned) graph[eq].count && sameWeight++ == 0){ 
						targetEdge = e;
						//causalizedUnknown = unknown;
					}
				}
				if(sameWeight == 1){
					/*TODO Makecausal*/
					remove_edge_from_array(targetEdge,toRemove);
				}
			}
		}else{
			ERROR("CausalizationStrategy::causalize:"
			      "Equation type not supported\n");		
		}
	}
}
