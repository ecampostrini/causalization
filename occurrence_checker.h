/* check if a variable occurs in an expression and if the variable is
 * an array it returns a list with the indexes in which occurs 
 * The results are saved in 
 */
#include <util/ast_util.h>
#include <util/symbol_table.h>
#include <mmo/mmo_class.h>
#include <causalize/causalize2/graph_definition.h>
#include <causalize/for_unrolling/for_index_iterator.h>
#include <boost/icl/discrete_interval.hpp>

class Occurrence_checker : public AST_Expression_Fold<bool>{
	public:
		Occurrence_checker(VarSymbolTable sTable);
		~Occurrence_checker();
		bool check_occurrence(VertexProperties variable, AST_Equation equation);
		list<EdgeProperties> getOccurrenceIndexes();
	private:
		/*methods*/
		virtual bool foldTraverseElement(AST_Expression);
		virtual bool foldTraverseElementUMinus(AST_Expression);
		virtual bool foldTraverseElement(bool, bool, BinOpType);
		void arrayOccurrence(AST_Expression_ComponentReference);
		pair<AST_Integer, AST_Integer> get_for_range(AST_Expression, VarSymbolTable);
		void add_generic_index(AST_Expression);
		/*fields*/
		//EqualExp *equalExp;
		VertexProperties variable;
		AST_Equation equation;
		list<EdgeProperties> edgeList;
		VarSymbolTable symbolTable;
		EvalExp *evaluator;

		set< pair<AST_Integer, AST_Integer> > genericIndexSet;
		boost::icl::discrete_interval<AST_Integer> forIndexInterval;
		set<int> simpleIndex;
};
