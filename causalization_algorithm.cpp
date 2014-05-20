#include <causalize/causalize2/causalization_algorithm.h>
#include <boost/graph/adjacency_list.hpp>
using namespace boost;

CausalizationStrategy::CausalizationStrategy(CausalizationGraph g){
	graph = g;
	equations1toN = (MMO_EquationList) newMMO_EquationList;
	equationsNto1 = (MMO_EquationList) newMMO_EquationList;
	equationDescriptors = new list<Vertex>();
	unknownDescriptors = new list<Vertex>();	
	CausalizationGraph::vertex_iterator vi, vi_end;
	equationNumber = unknownNumber = 0;
	for(tie(vi, vi_end) = vertices(graph); vi != vi_end; vi++){
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
CausalizationStrategy::causalize(){	


	
			
}
