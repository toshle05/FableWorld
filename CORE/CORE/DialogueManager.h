#pragma once
#include"Tree.h"
#include"Label.h"
#include"libs/TinyXml2/tinyxml2.h"
#include "Misc.h"
#include <vector>
#include <string>
#include "GameObjectManager.h"

/* This structure holds all the information needed for single node in the tree
m_nKey - this is the id of the node.
m_strText - the text of the dialogue
m_label - the label is need to detect user click on the text
m_vNodes - this is std::vector that holds the children of this node.The children can contain children and etc.
m_strQuest - holds the name of the quest */
struct DialogueNode {
	std::string	m_strText;
	Label* m_pLabel;
	std::vector<DialogueNode*> m_vNodes;
	std::string m_strQuest;
};

/* This is struct that holds all the information needed for one dialogue
m_strModel - This variable holds the name of the model. When clicking on this model this dialogue will appear
m_pTree - Holds the ierarchy of dialogue nodes
m_pCurrentDialogueNode - Holds the currender dialogue node, which childs are displayed on the screen
m_pClickedDialogueNode - Holds the dialogue node that is currently selected with mouse
m_bIsClickedDialogueNode - If true we know that some of the dialogue node is clicked. It is used in labelClicked
m_bIsStarted - If true the dilogue is started i.e. the user clicked on the root of the dialogue and can interact
m_bIsEnded - If true the dialogue ended. */
struct DialogueObject {
	std::string m_strModel;
	Tree* m_pTree;
	DialogueNode* m_pCurrentDialogueNode;
	DialogueNode* m_pClickedDialogueNode;
	bool m_bIsClickedDialogueNode;
	bool m_bIsStarted;
	bool m_bIsEnded;
};

class DialogueManager {
public:	
	void LoadDialogues(std::string strDialoguesFileName);
	void TraverseNodes(tinyxml2::XMLElement* xmlNode, Tree* pTree, DialogueNode* parentNode, DialogueNode* currentNode);
	void OnUpdate();
	//invokes onRender on every label in the tree
	void RenderDialogueTree(DialogueNode* pNode);
	void HideDialogueTree(DialogueNode* pNode);

	/* This is the function responsible for changing the current dialogue with another until the user reaches the end of the dialogue
	The function first checks if the dialogue is started. The dialogue starts when the user clicks on the root and his children are shown
	Then the current dialogue node is the root, his children are revealed and is executed the else if in this function, which
	checks if the user clicked on any if the shown children.If any of the children is clicked isClickedDialogueNode is changed to true
	and the clickedDialogueNode variable is changed to the clicked node. The dialogue changing later continues in labelClicked function
	when isClickedDialogueNode is true and the user releases the mouse botton this means that
	dialogue node is selected.
	Then if the node got quest the user receives it and the dialogue ends.
	Otherwise the current dialogue node is changed to the clicked one.
	Both labelClicked() and changeDialogue() functions are invoked in onUpdate() in Game.cpp
	Firstly labelClicked is inactive and after node is clicked labelClicked becomes active
	and changeDialogue inactive.This way we can detect single click om dialogue - in changeDialogue we detect if the user
	clicked on node and late in labelClicked we detect if the user released the mouse button.
	Click and release combination of mouse button ensures single click. */
	void UpdateDialogue(DialogueNode* pNode, DialogueObject& dialogue);
	
	//this function checks if the children of the passed node are hidden.Used in changeLabel(), labelClicked()
	bool AreChildrenHidden(DialogueNode* pNode);

	//add the passed dialogue object to modelDialogue map.
	//This map connects actual model in game(dwarf,robot, etc) with dialogue
	//so when the user clicks on the model the dialogue is displayed
	void AddDialogue(DialogueObject* dialogueObject);

	std::vector<DialogueObject*>& GetDialogues();

private:
	std::vector<DialogueObject*> m_dialogues;
	tinyxml2::XMLElement* m_pRoot;
	tinyxml2::XMLElement* m_pNode;
	tinyxml2::XMLElement* m_pDialogue;
};