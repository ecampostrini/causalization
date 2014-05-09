/*
*	This class provides the interface to build the causalization
* graph which is then going to be processed by the causalization
* algorithm. Since there may be more than one way of building it,
* We'll have a base abstract class and (maybe) several concrete
* classes depending on what kind of 
*/

#include <causalize/causalize2/graph_definition.h>


class GraphBuilder{
	public: 
		GraphBuilder(MMO_Class mmo_class);
		~GraphBuilder();
		virtual CausalizationGraph makeGraph() = 0;
}
