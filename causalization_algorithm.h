#include <causalize/causalize2/graph_definition.h>
#include <boost/icl/interval_set.hpp>
#include <mmo/mmo_class.h>

using namespace boost::icl;

class CausalizationStrategy{
	public:
		CausalizationStrategy(CausalizationGraph g);			
		void causalize();
		MMO_EquationList getEquations();
		void print();
	private:
		void remove_edge_from_array(Vertex, Edge);
		CausalizationGraph graph;
		AST_Integer equationNumber;
		AST_Integer unknownNumber;
		list<Vertex> *equationDescriptors, *unknownDescriptors;
		vector<CausalizedVar> equations1toN;
		vector<CausalizedVar> equationsNto1;
		void causalize1toN(const Vertex &unknown, const Vertex &equation, const Edge &e);
		void causalizeNto1(const Vertex &unknown, const Vertex &equation, const Edge &e);
		bool test_intersection(const Edge&, const Edge&);
};
