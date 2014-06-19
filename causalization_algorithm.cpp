#include <causalize/causalize2/causalization_algorithm.h>
#include <causalize/causalize2/graph_definition.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/icl/discrete_interval.hpp>
//#include <iostream>
//#include <string>
#include <sstream>

#define sz(a) int((a).size())

using namespace std;
using namespace boost;
using namespace boost::icl;

CausalizationStrategy2::CausalizationStrategy2(CausalizationGraph g){
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

bool 
CausalizationStrategy2::test_intersection(const Edge &e1, const Edge &e2){

	if(graph[e1].genericIndex.first > 1 || graph[e2].genericIndex.first > 1){
		//we transform the first and the last element of each interval
		//we represent e1 interval as [a,b] and e2 interval as [c,d]
		int a = first(graph[e1].indexInterval), b = last(graph[e1].indexInterval);
		int c = first(graph[e2].indexInterval), d = last(graph[e2].indexInterval);

		//DEBUG('g', "Testing intersection between: [%d, %d](%d, %d), [%d, %d](%d,%d)\n", a,b,graph[e1].genericIndex.first,
		//   graph[e1].genericIndex.second,c,d, graph[e2].genericIndex.first, graph[e2].genericIndex.second);
		
		if(graph[e1].genericIndex.first > 1){
			a = graph[e1].genericIndex.first * a + graph[e1].genericIndex.second;		
			b = graph[e1].genericIndex.first * b + graph[e1].genericIndex.second;
		}
		if(graph[e2].genericIndex.first > 1){
			c = graph[e2].genericIndex.first * c + graph[e2].genericIndex.second;
			d = graph[e2].genericIndex.first * d + graph[e2].genericIndex.second;		
		}
		//we check if we have an interval intersection between
		//[a,b] and [c,d]
		if(c == a || c == b	|| d == a || d == b){
			//we have a match in some extreme!
			return true;
		}
		else if((c < a && d > a) || (c > a && b > c)){
			if(d < b){
				int d_ = d - graph[e1].genericIndex.second; 
				if(d_ % graph[e1].genericIndex.first == 0){
					return true;
				}
			}else{
				int b_ = b - graph[e2].genericIndex.second;
				if(b_ % graph[e2].genericIndex.first == 0){
					return true;
				}
			}
		}
		return false;
	}
	return intersects(graph[e1].indexInterval, graph[e2].indexInterval);
}

void
CausalizationStrategy2::remove_edge_from_array(Vertex unknown, Edge targetEdge){
	assert(boost::icl::size(graph[targetEdge].indexInterval) != 0);
	//DEBUG('g', "Removing edge for unknown: %s\n", graph[unknown].variableName.c_str());
	CausalizationGraph::out_edge_iterator it, end, auxiliaryIter;
	tie(auxiliaryIter, end) = out_edges(unknown, graph);
	for(it = auxiliaryIter; it != end; it = auxiliaryIter){
		auxiliaryIter++;
		if(current_element(it) == targetEdge){continue;}
		if(test_intersection(targetEdge, current_element(it))){
			remove_edge(current_element(it), graph);
		}
	}
	remove_edge(targetEdge, graph);
}

void 
CausalizationStrategy2::causalize1toN(const Vertex &u, const Vertex &eq, const Edge &e){
	CausalizedVar c_var;
	c_var.unknown = graph[u];
	c_var.equation = graph[eq];
	c_var.edge = graph[e];
	equations1toN.push_back(c_var);
}

void 
CausalizationStrategy2::causalizeNto1(const Vertex &u, const Vertex &eq, const Edge &e){
	CausalizedVar c_var;
	c_var.unknown = graph[u];
	c_var.equation = graph[eq];
	c_var.edge = graph[e];
	equationsNto1.insert(begin(equationsNto1), c_var);		
}

bool
CausalizationStrategy2::causalize(){	
	assert(equationNumber == unknownNumber);
	if(equationDescriptors->empty()) return true;
	
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
					unknownNumber -= sz(graph[e].indexInterval);
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
				assert(sz(graph[e].indexInterval) == graph[eq].count);
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
					unknownNumber -= sz(graph[e].indexInterval);
					//the array may have more variables
					graph[unknown].count -= sz(graph[e].indexInterval);
					if(graph[unknown].count == 0){
						unknownDescriptors->remove(unknown);
					}
					equationDescriptors->erase(iter);
				}
			}
		}else{
			ERROR("CausalizationStrategy2::causalize:"
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
					if(e != e2 && (target(e, graph) == target(e2, graph) || test_intersection(e, e2))){
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
					unknownNumber -= sz(graph[e].indexInterval);
					equationDescriptors->remove(eq);
				}
			}
		}
	}
	if(numAcausalEqs == equationDescriptors->size()){
		//we have a LOOP or a FOR equation that we don't 
		//handle at least yet, so we resort to the previous
		//algorithm
		return false;
		//ERROR("Loop detected! We don't handle loops yet!\n");
	}
	return causalize();
}

string 
CausalizationStrategy2::getName(const VertexProperties &uk, const EdgeProperties &ed){
	stringstream name;

	if(ed.indexInterval.size() == 0){
		name << uk.variableName;		
	}else if(ed.indexInterval.size() == 1){
		name << uk.variableName << "[" << ed.genericIndex.first * first(ed.indexInterval) +  ed.genericIndex.second << "]";
	}else{
		name << uk.variableName << "[" << ed.genericIndex.first << "*i + " << ed.genericIndex.second << "]";		
	}

	if(uk.isState)
		return "der(" + name.str() + ")";

	return name.str();	
}

void
CausalizationStrategy2::print(){
	vector<CausalizedVar>::iterator it;
	for(it = equations1toN.begin(); it != equations1toN.end(); it++){
		string name;
		name = getName(it->unknown, it->edge);
		if(it->edge.indexInterval.size() > 1){
			interval_set<int>::iterator isi = it->edge.indexInterval.begin();
			cout << "(" << name <<	"," << *isi  << "," << it->equation.index << ")" << endl; 
		}else{
			cout << "(" << name << "," << it->equation.index << ")" << endl;		
		}
	}		
	for(it = equationsNto1.begin(); it != equationsNto1.end(); it++){
		string name;
		name = getName(it->unknown, it->edge);
		if(it->edge.indexInterval.size() > 1){
			interval_set<int>::iterator isi = it->edge.indexInterval.begin();
			cout << "(" << name <<	"," << *isi << "," << it->equation.index << ")" << endl; 
		}else{
			cout << "(" << name << ", " << it->equation.index << ")" << endl;		
		}
	}
}
