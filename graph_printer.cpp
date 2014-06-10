#include <causalize/causalize2/graph_definition.h>
#include <causalize/causalize2/graph_printer.h>

#include <iostream>
#include <fstream>
#include <sstream>
//#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>

using namespace boost;
using namespace boost::icl;
#define MAKE_SPACE for(int __i=0; __i<depth; __i++) stri << " ";
#define TAB_SPACE 2
#define INSERT_TAB depth += TAB_SPACE;
#define DELETE_TAB depth -= TAB_SPACE;

GraphPrinter::GraphPrinter(const CausalizationGraph &g)
{
	graph = g;
	CausalizationGraph::vertex_iterator vi, vi_end;
	for(tie(vi, vi_end) = vertices(graph); vi!= vi_end; vi++){
		if(graph[*vi].type == E){
			equationDescriptors.push_back(*vi);
		}else{
		unknownDescriptors.push_back(*vi);		
		}
	}
}

string
GraphPrinter::printGraph(){
	stringstream stri;
	ofstream out("grafo.dot");
	int depth = 0;

	stri << "graph G{" << endl;
	INSERT_TAB
		MAKE_SPACE
		stri << "subgraph cluster0{" << endl;
		INSERT_TAB
			MAKE_SPACE
			stri << "label = \"Ecuaciones\";" << endl;
			MAKE_SPACE
			stri << "edge [style=invis];" << endl;
			MAKE_SPACE
			for(list<Vertex>::iterator it=equationDescriptors.begin(); it!=equationDescriptors.end(); it++){
				list<Vertex>::iterator aux = it;
				aux++;
				stri << graph[*it].index;
				if((aux) != equationDescriptors.end()){
					stri << " -- ";		
				}else{
					stri << ";" << endl;		
				}
			}
		DELETE_TAB
		MAKE_SPACE
		stri << "}" << endl;
	DELETE_TAB


	INSERT_TAB
		MAKE_SPACE
		stri << "subgraph cluster1{" << endl;
		INSERT_TAB
			MAKE_SPACE
			stri << "label = \"Incognitas\";" << endl;
			MAKE_SPACE
			stri << "edge [style=invis];" << endl;
			MAKE_SPACE
			for(list<Vertex>::iterator it=unknownDescriptors.begin(); it!=unknownDescriptors.end(); it++){
				list<Vertex>::iterator aux = it;
				aux++;
				//if(graph[*it].isState){
				//	stri << "der(" << graph[*it].variableName << ")";
				//}else{
				//	stri << graph[*it].variableName;		
				//}
				stri << graph[*it].variableName;
				if((aux) != unknownDescriptors.end()){
					stri << " -- ";		
				}else{
					stri << ";" << endl;		
				}
			}
		DELETE_TAB
		MAKE_SPACE
		stri << "}" << endl;
	DELETE_TAB

	INSERT_TAB
		MAKE_SPACE
		stri << "edge [constraint=false];" << endl;
		for(list<Vertex>::iterator  eq_it = equationDescriptors.begin(); eq_it != equationDescriptors.end(); eq_it++){
			CausalizationGraph::out_edge_iterator ei, ei_end;
			for(tie(ei, ei_end) = out_edges(*eq_it, graph); ei != ei_end; ei++){
				Vertex unknown = target(*ei, graph);
				MAKE_SPACE;
				string name;
				//if(graph[unknown].isState){
				//	name = "der(" + graph[unknown].variableName + ")";		
				//}else{
				//	name = graph[unknown].variableName;
				//}
				name = graph[unknown].variableName;
				stri << graph[*eq_it].index << " -- " << name;
				if(graph[*ei].indexInterval.size() == 1){
					stri << "[label = \"[" << graph[*ei].genericIndex.first * first(graph[*ei].indexInterval) + graph[*ei].genericIndex.second  << "]\"]";
				}
				else if (graph[*ei].indexInterval.size() > 1){
					interval_set<int>::iterator isi = graph[*ei].indexInterval.begin();
					stri << " [label = \""<< *isi ;
					stri << ", " << graph[*ei].genericIndex.first << " * i + " << graph[*ei].genericIndex.second  << "\"]";
				}
				stri << ";" << endl;
			}
		}
	DELETE_TAB
	stri << "}" << endl;
	out << stri.str();
	out.close();
	cout << "Archivo gafo.dot generado" << endl;
	return stri.str();
}
