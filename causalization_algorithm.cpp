#include <causalize/causalize2/causalization_algorithm.h>
#include <causalize/causalize2/graph_definition.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/icl/discrete_interval.hpp>

using namespace std;
using namespace boost;
using namespace boost::icl;

CausalizationStrategy::CausalizationStrategy(CausalizationGraph g){
	graph = g;
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
			unknownNumber += ( graph[current_element].count == 0 ) ? 1 : graph[current_element].count;
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
CausalizationStrategy::remove_edge_from_array(Vertex unknown, Edge currentEdge){
	assert(boost::icl::size(graph[currentEdge].indexInterval) != 0);
	interval_set<int> toRemove;
	CausalizationGraph::out_edge_iterator it, end;
	tie(it, end) = out_edges(unknown, graph);
	while(it != end){
		if(current_element(it) == currentEdge) continue;
		if(!intersects(graph[current_element(it)].indexInterval, graph[currentEdge].indexInterval)) continue;
		Vertex eq = target(*it, graph);
		if(graph[eq].equation->equationType() == EQFOR){
			remove_edge(current_element(it), graph);	
		}
		else{
			graph[current_element(it)].indexInterval -= graph[currentEdge].indexInterval;
		}
		it++;
	}
	//remove_edge(currentEdge, graph);
}

void 
CausalizationStrategy::causalize1toN(Vertex u, Vertex eq, Edge e){
	CausalizedVar c_var;
	c_var.unknown = graph[u];
	c_var.equation = graph[eq];
	c_var.edge = graph[e];
	equations1toN.push_back(c_var);
}

void 
CausalizationStrategy::causalizeNto1(Vertex u, Vertex eq, Edge e){
	CausalizedVar c_var;
	c_var.unknown = graph[u];
	c_var.equation = graph[eq];
	c_var.edge = graph[e];
	equationsNto1.push_back(c_var);		
}

void
CausalizationStrategy::causalize(){	
	assert(equationNumber == unknownNumber);
	if(equationDescriptors->empty()) return;

	list<Vertex>::iterator iter;
	foreach(iter, equationDescriptors){
		Vertex eq = current_element(iter);
		EquationType eqType = graph[current_element(iter)].eqType;
		if(eqType == EQEQUALITY){
			//here we use the classic version of the algorithm
			if(out_degree(eq, graph) == 1){
				Edge e = *out_edges(eq, graph).first;
				Vertex unknown = target(e,graph);
				if (graph[unknown].count == 0){
					//its a regular variable
					assert(boost::icl::is_empty(graph[e].indexInterval));
					causalize1toN(unknown, eq, e);
					remove_out_edge_if(unknown, boost::lambda::_1 != e, graph);
					//remove_edge(e, graph);
					equationNumber--;
					unknownNumber--;
					equationDescriptors->erase(iter);
					unknownDescriptors->remove(unknown);
				}else{
					//its an array
					assert(boost::icl::size(graph[e].indexInterval) == 1);
					//causalize1toN(unknown, eq, graph[e].indexInterval);
					remove_edge_from_array(unknown, e);
					equationNumber--;
					unknownNumber -= graph[e].indexInterval.size();
					equationDescriptors->erase(iter);
					if(graph[unknown].count == 0){
						unknownDescriptors->remove(unknown);
					}
				}
			}else if(out_degree(eq, graph) == 0){
				ERROR("Problem is singular, not supported yet\n");
			}
		}/*else if(eqType == EQFOR){
			if(out_degree(eq,graph) == 1){
				Edge e = *out_edges(eq, graph).first;
				Vertex unknown = target(e, graph);
				if(graph[e].indexes.empty()){
					//its a regular variable and we just solve 
					//arrays in the FOR
				}else{
					//only one variable in the FOR, we causalize it
					//TODO MAKECAUSAL
					remove_edge_from_array(unknown, e);
					equationNumber -= graph[eq].count;
					unknownNumber -= graph[eq].count; //the array may have more variables 
					if(unknownNumber == 0){
						unknownDescriptors->remove(unknown);
					}
					equationDescriptors->erase(iter);
				}
			}else{
				//if only one of the edges has weight == size of range
				// then thats the one we are causalizing 
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
					//TODO Makecausal
					remove_edge_from_array(targetEdge,toRemove);
				}
			}
		}else{
			ERROR("CausalizationStrategy::causalize:"
			      "Equation type not supported\n");		
		}*/
	}
	
	//now we process the unknowns' side
	
	foreach(iter, unknownDescriptors){
		Vertex unknown = current_element(iter);
		ERROR_UNLESS(out_degree(unknown, graph) != 0, 
			"Problem is singular, not supported yet\n");
		if(out_degree(unknown, graph) == 1){
			Edge e = *out_edges(unknown, graph).first;					
			Vertex eq = target(e, graph);
			//causalizeNto1(unknowns, eq, e);
			remove_out_edge_if(eq, boost::lambda::_1 != e, graph);
			//remove_edge(e, graph);
			equationNumber -= graph[eq].count;
			unknownNumber -= (graph[unknown].count == 0) ? 1 : graph[unknown].count;
			unknownDescriptors->erase(iter);
			equationDescriptors->remove(eq);
		}else{
			CausalizationGraph::out_edge_iterator it, end;
			tie(it, end) = out_edges(unknown, graph);
			while(it != end){
				Edge e = *it;
				CausalizationGraph::out_edge_iterator _it, _end;
				tie(_it, _end) = out_edges(unknown, graph);
				while(_it != _end){
					Edge e2 = *_it;
					if(intersects(graph[e].indexInterval, graph[e2].indexInterval)){
						break;				
					}
						_it++;
				}
				if(_it == _end){
					//there is no intersection, we causalize it
					Vertex eq = target(e, graph);
					//causalizeNto1(unknown, eq, e);
					remove_out_edge_if(eq, boost::lambda::_1 != e, graph);
					equationNumber -= graph[eq].count;
					unknownNumber -= graph[e].indexInterval.size();
					equationDescriptors->remove(eq);
				}
				it++;
			}
		}
	}
}
