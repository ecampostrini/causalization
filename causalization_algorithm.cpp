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
	CausalizationGraph::out_edge_iterator it, end, auxiliaryIter;
	tie(auxiliaryIter, end) = out_edges(unknown, graph);
	for(it = auxiliaryIter; it != end; it = auxiliaryIter){
		auxiliaryIter++;
		if(current_element(it) == currentEdge){continue;}
		if(graph[current_element(it)].genericIndex.first > 1 || graph[currentEdge].genericIndex.first > 1){
			//we transform the first and the last element of each interval
			//we represent [a,b] for targetEdge as [target_first, target_last]
			//and [a,b] for currentEdge as [current_first, current_last]
			Edge cEdge = current_element(it);
			int a = first(graph[currentEdge].indexInterval);
			int b = last(graph[currentEdge].indexInterval);
			int c = first(graph[cEdge].indexInterval);
			int d = last(graph[cEdge].indexInterval);
			if(graph[currentEdge].genericIndex.first > 1){
				a = graph[currentEdge].genericIndex.first * a + graph[currentEdge].genericIndex.second;		
				b = graph[currentEdge].genericIndex.first * b + graph[currentEdge].genericIndex.second;
			}
			if(graph[cEdge].genericIndex.first > 1){
				c = graph[cEdge].genericIndex.first * c + graph[cEdge].genericIndex.second;
				d = graph[cEdge].genericIndex.first * d  + graph[cEdge].genericIndex.second;		
			}
			//we check if we have an interval intersection between
			//[a,b] (target edge) and [c,d] (current edge)
			if(c == a || c == b	|| d == a || d == b){
				//we have intersection in some extreme!
				remove_edge(cEdge, graph);
			}
			else if((c < a && d > a) || (c > a && b > c)){
				//a > c and d > a
				if(d < b){
					int d_ = d - graph[currentEdge].genericIndex.second; 
					if(d_ % graph[currentEdge].genericIndex.first == 0){
						remove_edge(cEdge, graph);		
					}
				}else{
					int b_ = b - graph[cEdge].genericIndex.second;
					if(b_ % graph[cEdge].genericIndex.first == 0){
						remove_edge(cEdge, graph);		
					}
				}
			}
			
			//else if(c > a && b > c)
			//	if(d < b){
			//				
			//	}else{
			//			
			//	}
			//	a < c and b > c
			else{
				//there is no intersection, we jump to the next edge of the array 
				//continue;
			}
			continue;
		}
		if(!intersects(graph[current_element(it)].indexInterval, graph[currentEdge].indexInterval)){continue;}
		Vertex eq = target(*it, graph);
		//cout << graph[current_element(it)].indexInterval << endl;
		//cout << "a: " << graph[current_element(it)].genericIndex.first;
		//cout << ", indexInterval last: " << last(graph[current_element(it)].indexInterval) << endl;
		remove_edge(current_element(it), graph);
		//if(graph[eq].equation->equationType() == EQFOR){
		//	remove_edge(current_element(it), graph);	
		//}
		//else{
		//	graph[current_element(it)].indexInterval -= graph[currentEdge].indexInterval;
		//	if(boost::icl::is_empty(graph[current_element(it)].indexInterval)){
		//		remove_edge(current_element(it), graph);
		//	}
		//}
	}
	remove_edge(currentEdge, graph);
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
	
	list<Vertex>::size_type numAcausalEqs = equationDescriptors->size();
	list<Vertex>::iterator iter, auxiliaryIter;
	auxiliaryIter = equationDescriptors->begin();
	for(iter = auxiliaryIter; iter != equationDescriptors->end(); iter = auxiliaryIter){
		auxiliaryIter++;
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
					remove_edge(e, graph);
					equationNumber--;
					unknownNumber--;
					equationDescriptors->erase(iter);
					unknownDescriptors->remove(unknown);
				}else{
					//its an array
					assert(boost::icl::size(graph[e].indexInterval) == 1);
					causalize1toN(unknown, eq, e);
					remove_edge_from_array(unknown, e);
					equationNumber--;
					unknownNumber -= graph[e].indexInterval.size();
					equationDescriptors->erase(iter);
					graph[unknown].count--;
					if(graph[unknown].count == 0){
						unknownDescriptors->remove(unknown);
					}
				}
			}else if(out_degree(eq, graph) == 0){
				ERROR("Problem is singular, not supported yet\n");
			}
		}else if(eqType == EQFOR){
			if(out_degree(eq,graph) == 1){
				//only one edge, we causalize it
				Edge e = *out_edges(eq, graph).first;
				Vertex unknown = target(e, graph);
				if(boost::icl::is_empty(graph[e].indexInterval)){
					//its a regular variable and we just causalize
					//arrays in the FOR
					ERROR("Trying to causalize a non-array variable with a FOR equation\n");
				}else{
					//only one variable in the FOR, we causalize it
					causalize1toN(unknown, eq, e);
					remove_edge_from_array(unknown, e);
					equationNumber -= graph[eq].count;
					unknownNumber -= graph[e].indexInterval.size();
					//the array may have more variables
					graph[unknown].count -= graph[e].indexInterval.size();
					if(graph[unknown].count == 0){
						unknownDescriptors->remove(unknown);
					}
					equationDescriptors->erase(iter);
				}
			}
			/*
			else{
				//if only one of the edges has weight == size of range
				// then thats the one we are causalizing 
				CausalizationGraph::out_edge_iterator begin, end, it;
				Edge targetEdge; 
				Vertex causalizedUnknown;
				AST_Integer sameWeight = 0;
				vector<Edge> toRemove;
				for(boost::tie(begin, end) = out_edges(eq, graph), it = begin; it != end && sameWeight <= 1; it++){
					Edge e = *it;
					Vertex unknown = target(e, graph);
					//DEBUG('g', "Variable %s, indexInterval size %d\n", graph[unknown].variableName.c_str(), graph[e].indexInterval.size());
					if(graph[e].indexInterval.size() == (unsigned) graph[eq].count && sameWeight++ == 0){ 
						targetEdge = e;
						causalizedUnknown = unknown;
					}
					if(e != targetEdge && sameWeight <= 1){
						DEBUG('g', "toRemove: %s\n", graph[unknown].variableName.c_str());
						toRemove.push_back(e);		
					}
				}
				if(sameWeight == 1){
					if(toRemove.empty()){
						//we have to arrange the equations in an 'executable' order
						causalize1toN(causalizedUnknown, eq, targetEdge);
					}
					else{
						causalizeNto1(causalizedUnknown, eq, targetEdge);
					}
					remove_edge_from_array(causalizedUnknown, targetEdge);
					equationNumber -= graph[eq].count;
					unknownNumber -= graph[targetEdge].indexInterval.size();
					graph[causalizedUnknown].count -= graph[targetEdge].indexInterval.size();
					if(graph[causalizedUnknown].count == 0)
						unknownDescriptors->remove(causalizedUnknown);
					equationDescriptors->erase(iter);
					for(vector<Edge>::iterator it = toRemove.begin(); it != toRemove.end(); it++){
						Edge e = *it;
						remove_edge(e, graph);
					}
				}
			}*/
		}else{
			ERROR("CausalizationStrategy::causalize:"
			      "Equation type not supported\n");		
		}
	}
	
	//now we process the unknowns' side
	auxiliaryIter = unknownDescriptors->begin();
	for(iter = auxiliaryIter; iter != unknownDescriptors->end(); iter = auxiliaryIter){
		auxiliaryIter++;
		Vertex unknown = current_element(iter);
		ERROR_UNLESS(out_degree(unknown, graph) != 0, 
			"Problem is singular, not supported yet\n");
		if(out_degree(unknown, graph) == 1){
			Edge e = *out_edges(unknown, graph).first;					
			Vertex eq = target(e, graph);
			causalizeNto1(unknown, eq, e);
			remove_out_edge_if(eq, boost::lambda::_1 != e, graph);
			remove_edge(e, graph);
			equationNumber -= graph[eq].count;
			unknownNumber -= (graph[unknown].count == 0) ? 1 : graph[unknown].count;
			unknownDescriptors->erase(iter);
			equationDescriptors->remove(eq);
		}else if (graph[unknown].count != 0){
			//we make sure we have an array variable
			CausalizationGraph::out_edge_iterator it, end, auxiliaryIter2;
			tie(auxiliaryIter2, end) = out_edges(unknown, graph);
			for(it = auxiliaryIter2; it != end; it = auxiliaryIter2){
				//we check if the indexes of the edge don't appear in other equations
				auxiliaryIter2++;
				Edge e = *it;
				CausalizationGraph::out_edge_iterator _it, _end;
				tie(_it, _end) = out_edges(unknown, graph);
				while(_it != _end){
					Edge e2 = *_it;
					if(e != e2 && intersects(graph[e].indexInterval, graph[e2].indexInterval)){
						break;				
					}
					_it++;
				}
				if(_it == _end){
					//there is no intersection, we causalize it
					Vertex eq = target(e, graph);
					causalizeNto1(unknown, eq, e);
					remove_out_edge_if(eq, boost::lambda::_1 != e, graph);
					remove_edge(e, graph);
					equationNumber -= graph[eq].count;
					unknownNumber -= graph[e].indexInterval.size();
					equationDescriptors->remove(eq);
				}
			}
		}
	}
	if(numAcausalEqs == equationDescriptors->size()){
		ERROR("Loop detected! We don't handle loops yet!\n");
	}
	causalize();
}

void
CausalizationStrategy::print(){
	vector<CausalizedVar>::iterator it;
	
	for(it = equations1toN.begin(); it != equations1toN.end(); it++){
		cout << "(" << it->unknown.variableName <<	", " << it->equation.index << ", " << it->edge.indexInterval << ")" << endl;
	}		
	for(it = equationsNto1.begin(); it != equationsNto1.end(); it++){
		cout << "(" << it->unknown.variableName <<	", " << it->equation.index << ", " << it->edge.indexInterval << ")" << endl;
	}
}
