/* check if a variable occurs in an expression and if the variable is
 * an array it returns a list with the indexes in which occurs 
 * The results are saved in 
 */
#include <util/ast_util.h>
#include <util/symbol_table.h>
#include <mmo/mmo_class.h>

class Occurrence_checker : public AST_Expression_Fold<bool>{
	public:
		Occurrence_checker(VarSymbolTable sTable){
			//symbolTable = sTable;
			equalExp = new EqualExp(sTable)};
		bool check_occurrence(AST_String varName,bool isState ,AST_Expression exp);
		list<AST_IntegerSet>* getOccurrenceIndexes();
	private:
		virtual bool foldTraverseElement(AST_Expression);
		virtual bool foldTraverseElementUMinus(AST_Expression);
		virtual bool foldTraverseElement(bool, bool, BinOpType);
		EqualExp *equalExp;
		AST_Expression exp;
		//VarSymbolTable symbolTable;
		list<AST_IntegerSet>* occurrenceSetList;
		AST_String varName;
};
