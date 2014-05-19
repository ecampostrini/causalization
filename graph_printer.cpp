#include <causalize/causalize2/graph_definition.h>
#include <causalize/causalize2/graph_printer.h>

#include <iostream>
#include <boost/graph/graphviz.hpp>

using namespace boost;

GraphPrinter::GraphPrinter(CausalizationGraph g)
{
	graph = g;
	/*TODO exception handling onopening file
	outFile.open(oFile);
	if(!outFile.is_open()){
		ERROR("GraphPrinter: error while opening the output file\n");
	}*/
}

void
GraphPrinter::printGraph(){
  	//dynamic_properties dp;
  	//dp.property("id", get(vertex_name, g));
  	//dp.property("weight", get(edge_weight, g));

  	//write_graphviz(std::cout, graph, default_writer(), default_writer(), default_writer());//, dp, std::string("id"));
			
	//boost::write_graphviz(std::cout, graph);
	
}
