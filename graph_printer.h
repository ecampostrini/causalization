#include <causalize/causalize2/graph_definition.h>
//#include <ostream>

class GraphPrinter{
	public:
		GraphPrinter(CausalizationGraph g);
		void printGraph();
	private:
		CausalizationGraph graph;
		//ofstream outFile;

};
