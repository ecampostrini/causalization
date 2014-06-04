#include <causalize/causalize2/graph_definition.h>
//#include <ostream>

class GraphPrinter{
	public:
		GraphPrinter(CausalizationGraph g);
		string printGraph();
	private:
		CausalizationGraph graph;
		list<Vertex> equationDescriptors;
		list<Vertex> unknownDescriptors;
		//ofstream outFile;

};
