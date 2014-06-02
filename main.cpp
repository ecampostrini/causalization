
/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

#include <parser/parse.h>
#include <util/debug.h>
#include <mmo/mmo_class.h>
#include <ast/class.h>
#include <util/symbol_table.h>

/*this includes are for a quick test*/
#include <causalize/causalize2/graph_builder.h>
#include <causalize/causalize2/graph_definition.h>
#include <causalize/causalize2/graph_printer.h>
#include <causalize/causalize2/causalization_algorithm.h>

int main(int argc, char** argv){
	int opt;
	int r;
	AST_Class modelica_class;
	
	while((opt = getopt(argc, argv, "d:")) != -1){
		switch(opt){
			case 'd':
				if(optarg != NULL && isDebugParam(optarg)){
					debugInit(optarg);		
				}else{
					ERROR("Command line option d requires an argument\n");
				}
				break;
		}		
	}
	
	if(optind < argc){
		modelica_class = parseClass(argv[optind],&r);
	}else{ 
		/* si no se especifico un archivo leo de stdin*/
		modelica_class = parseClass("", &r);	
	}
	if(r!=0){
		return -1;
	}
	
	/* creamos la clase MicroModelica */
	TypeSymbolTable ty = newTypeSymbolTable();
	MMO_Class mmo_class = newMMO_Class(modelica_class, ty); 

	MMO_EquationListIterator mmo_iterator;
	MMO_EquationList mmo_eqs = mmo_class->getEquations();

	DEBUG('c', "Acausal equations\n");
	foreach(mmo_iterator, mmo_eqs){
		DEBUG('c', "%s", current_element(mmo_iterator)->print().c_str());
	}	
	ReducedGraphBuilder *gb = new ReducedGraphBuilder(mmo_class);
	CausalizationGraph g = gb->makeGraph();
	CausalizationStrategy *cs = new CausalizationStrategy(g);
	cs->causalize();
	cs->print();
	//GraphPrinter gp(g);
	//gp.printGraph();
	return 0;
}
