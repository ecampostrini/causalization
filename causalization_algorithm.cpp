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
CausalizationStrategy::remove_edge_from_array(Edge targetEdge, map<Edge, Vertex> toRemove){
		
}

void
CausalizationStrategy::remove_edge_from_array(Vertex unknown, Edge currentEdge){
	assert(boost::icl::size(graph[currentEdge].indexInterval) != 0);
	CausalizationGraph::out_edge_iterator begin, end;
	tie(it, end) = out_edges(unknown, graph);
	while(it != end){
		if(current_element(it) == currentEdge) continue;
		if(!intersects(graph[current_element(it)].indexInterval, graph[currentEdge].indexInterval)) break;
		
		//int oldSize = graph[current_element(it)].indexInterval.size();
		graph[current_element(it)].indexInterval -= graph[currentEdge].indexInterval;
		graph[unknown].count -= graph[currentEdge].indexInterval.size();


		Vertex eq = target(current_element(it), graph);
		//if the equation connected to the edge pointed by 'it' is a FOR
		if(graph[eq].equation->equationType == EQFOR){
			//we remove this connection since we are not using it to solve the current indexes
			graph[unknown].count -= graph[current_element(it)].indexInterval.size();
			remove_edge(eq, unknown, graph);
		}
		else if(intersects(graph[current_element(it).indexInterval], graph[currentEdge].indexInterval)){
			int oldSize = graph[current_element(it)].indexInterval.size();
			graph[current_element(it)].indexInterval -= graph[currentEdge].indexInterval;
			graph[unknown].count -= (oldSize - graph[current_element(it)].indexInterval.size());
		}
		it++;
	}
}

/*
void causalize1toN(Vertex unknown, Vertex equation, boost::icl::discrete_interval<int> indexInterval){
	CausalizedVar c_var;
	c_var.variableName = unknown.variableName;
	c_var.isState = unknown.isState;
	c_var.index = arrayPos;
	c_var.equation = equation.equation;
	//c_var.indexInterval = boost::icl::construct< boost::icl::discrete_interval<int> > (0, 0, boost::icl::interval_bounds::open());
	c_var.indexInterval = indexInterval;
	equations1toN.push_back(c_var);
}
*/

void
CausalizationStrategy::causalize(){	
	/*list<Vertex>::iterator iter;
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
					remove_out_edge_if(unknown, boost::lambda::_1 != e, graph);
					//causalize1toN(unknown, eq, graph[e].indexInterval);
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
					unknownNumber--;
					if(--graph[unknown].count == 0){
						unknownDescriptors->remove(unknown);
					}
					equationDescriptors->erase(iter);
				}
			}else if(out_degree(eq, graph) == 0){
				ERROR("Problem is singular, not supported yet\n");
			}*/
		/*}else if(eqType == EQFOR){
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
		}
	}*/
}
