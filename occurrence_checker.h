/* check if a variable occurs in an expression and if the variable is
 * an array it returns a list with the indexes in which occurs */
#include <util/ast_util.h>
#include <util/symbol_table.h>
#include <mmo/mmo_class.h>

class Occurrence_checker : public AST_Expression_Fold<bool>{
	public:
		Occurrence_checker(AST_Expression exp, VarSymbolTable symbolTable);
		bool check_occurrence();
		list<AST_Integer>* getOccurrenceIndexes();
	private:
		virtual bool foldTraverseElement(AST_Expression);
		virtual bool foldTraverseElementUMinus(AST_Expression);
		virtual bool foldTraverseElement(bool, bool, BinOpType);
		AST_Expression exp;
		VarSymbolTable symbolTable;
		list<AST_Integer>* occurrenceSetList;
};
