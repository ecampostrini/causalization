/* check if a variable occurs in an expression and if the variable is
 * an array it returns a list with the indexes in which occurs 
 * The results are saved in 
 */
#include <util/ast_util.h>
#include <util/symbol_table.h>
#include <mmo/mmo_class.h>
#include <causalize/causalize2/graph_definition.h>

class Occurrence_checker : public AST_Expression_Fold<bool>{
	public:
		Occurrence_checker(VarSymbolTable sTable);
		bool check_occurrence(VertexProperties variable, AST_Equation equation);
		list<EdgeProperties*>* getOccurrenceIndexes();
	private:
		virtual bool foldTraverseElement(AST_Expression);
		virtual bool foldTraverseElementUMinus(AST_Expression);
		virtual bool foldTraverseElement(bool, bool, BinOpType);
		EqualExp *equalExp;
		VertexProperties variable;
		AST_Equation equation;
		//VarSymbolTable symbolTable;
		list<EdgeProperties*>* occurrenceSetList;
};
