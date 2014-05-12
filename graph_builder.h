/*
* This class provides the interface to build the causalization
* graph which is then going to be processed by the causalization
* algorithm. Since there may be more than one way of building it
* we'll have a base abstract class and (maybe) several concrete
* implementations.
*/

#include <causalize/causalize2/graph_definition.h>


class GraphBuilder{
	public: 
		GraphBuilder(MMO_Class mmo_class){
			eqList = mmo_class->getEquations();		
		};
		~GraphBuilder();
		virtual CausalizationGraph makeGraph() = 0;
	private:
		CausalizationGraph graph;
		MMO_EquationList eqList;
};

class ReducedGraphBuilder : public GraphBuilder{
	public:
		CausalizationGraph makeGraph();
	private:
		EquationType getType(MMO_Equation eq);
		int processForRange(MMO_Equation);
};
