#include <causalize/causalize2/graph_builder.h>

CausalizationGraph :: ReducedGraphBuilder::makeGraph(){
	/* Create nodes for the Equations*/
	MMO_EquationListIterator it;
	foreach(it, eqList){
		VertexProperties *vp = new VertexProperties;
		vp->type = E;
		vp->equation = current_element(it);
		vp->eqType = getType(current_element(it));
		if(vp->eqType == EQFOR){
			vp->count = processForRange(vp->equation);		
		}else{
			vp->count = 1;		
		}
	}
	
	/* Create nodes for the unkowns */
	/* Create the edges */
}
