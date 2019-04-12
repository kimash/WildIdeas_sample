//
//  WildIdeasPlacemat.h
//  
//
//  Created by Kim Ash on 4/29/15.
//
//
#pragma once
#include "ofMain.h"
#include "GenericPlacemat.h"
#include "WildIdeasHeader.h"
#include "AttractLoopHeader.h"
#include "ImagePairPlaylist.h"
#include "Boundary.h"
#include "ActiveTextureButton.h"
#include "DialogBoxOverlay.h"
#include "CanvasOverlay.h"
class GComponent;
class ImageChanger;
class Canvas;
class MoveableButtonPanel;
class BrowserPanel;
class SketchBrowser;
class SketchViewer;
class CanvasDialogBox;
class PulsateButton;
class TimeoutDialog;

class Placemat: public GenericPlacemat, public GComponent {
public:
	enum State {
		ATTRACT,
		BROWSE,
		TRANSITION,
		DRAWING,
		REVERSE_TRANSITION,
		SHRINK_UI,
		GROW_UI
	};

	State getState() {return state;}
	void setState(State s);
	void updateState();
	void advancePlaylist();
	void setNewPair(ImagePairPlaylist::ImagePair*);

	void setup(float x_, float y_, float w_, float h_);
	void draw();
	~Placemat();
	
	//touch callbacks
	void onAddDesignBtnTouch(TouchEvent& event);
	void onBrowserAddDesignBtnTouch(TouchEvent& event);
	void onPostBtnTouch(TouchEvent& event);
	void onBrowseBtnTouch(TouchEvent& event);
	void onShuffleBtnTouch(TouchEvent& event);
	void onBrowserPanelTouch(TouchEvent& event);
	void onSketchViewerTouch(TouchEvent& event);
	void onImageChangerTouch(TouchEvent& event);
	void onSketchBtnClick(int& sketchIndex);
	void onDBoxBackToDesignClick(TouchEvent& event);
	void onDBoxStartOverClick(TouchEvent& event);
	
	//animation callbacks
	void onRotateAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onImageSlideDownDone(ofxAnimatable::AnimationEvent& animEvent);
	void onBPTransitionDone(ofxAnimatable::AnimationEvent& animEvent);	//TODO: rename
	void onTogetherAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onSketchViewerGrowAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onSketchViewerShrinkAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onButtonGrowAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onHeaderAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onShrinkButtonAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onDialogBoxTimeoutDone(ofxAnimatable::AnimationEvent& animEvent);
	void onPlacematTimeoutDone(ofxAnimatable::AnimationEvent& animEvent);

	//canvas coachmark animation
	void startCanvasCoachmarkAnim();
	void drawCanvasCoachmarkAnim();

	//after save to pen
	void showMyCreationInBrowser();
	
	//GenericPlacemat functions
	ofVec2f getPlacematPosition() {return getGlobalPosition();}
    ofRectangle getPlacematRect() {return getGlobalBounds();}
	void resetTimeoutTimer();
	void showMyCollectionsForTicket(string ticketID) {}
	void addChild(GComponent* child) {GComponent::addChild(child);}

	//animations
	ofxAnimatableFloat canvasCoachmarkAnim;
	ofxAnimatableFloat sketchViewerGrowAnim;
	ofxAnimatableFloat sketchViewerShrinkAnim;
	ofxAnimatableFloat buttonGrowAnim;
	ofxAnimatableFloat buttonShrinkAnim;
	ofxAnimatableFloat attractBtnShrinkAnim;

	//animation functions
	void startSketchViewerGrowAnim();
	void drawSketchViewerGrowAnim();
	void startSketchViewerShrinkAnim();
	void drawSketchViewerShrinkAnim();
	void startButtonGrowAnim();
	void drawButtonGrowAnim();
	void startShrinkButtonAnim();
	void drawShrinkButtonAnim();
	void startAttractBtnShrinkAnim();
	void drawAttractBtnShrinkAnim();
	

protected:
	float x, y, w, h;
	State state;
	State nextState;
	State prevState;
	ImagePairPlaylist::ImagePair* imagePair;

	//Canvas
	Canvas* mCanvas;
	MoveableButtonPanel* buttonPanel;
	Boundary* canvasBoundary;
	CanvasDialogBox* dialogBox;
	DialogBoxOverlay* overlayForDB;
	CanvasOverlay* canvasOverlay;
	ofTexture* canvasCoachmark;
	bool hasSketchToPost;
	
	//last post info for comparison - can't allow double posts
	int lastPostNumPaths;
	int finalPathCubics;	//number of cubic paths in last path posted
	int finalPathLines;		//number of lines in last path posted
	ofVec2f finalPathStartPt;	//start point of last path posted

	//timeout dialog box - are you still there?
	TimeoutDialog* timeoutDialog;

	//Attract Loop
    ImageChanger* imageChanger;
	int playlistIndex;
	SketchViewer* sketchViewer;
	bool svgShown;
	int svgCount;

	//Headers
	WildIdeasHeader* header;
	AttractLoopHeader* categoryHeader;

	//Idea Browser
	BrowserPanel* browserPanel;
	SketchBrowser* browser;

	//Buttons
	PulsateButton* addDesignButton;
	ActiveTextureButton* browserAddDesignButton;
	PulsateButton* shuffleButton;
	ActiveTextureButton* postButton;
	ActiveTextureButton* browseButton;

	//button textures that switch out in different situations
	ofTexture* addYourDesignTex;
	ofTexture* addYourDesignActiveTex;
	ofTexture* addYourDesignAltTex;
	ofTexture* backToDesignTex;
	ofTexture* backToDesignActiveTex;
	ofTexture* shuffleTex;
	ofTexture* shuffleActiveTex;
	ofTexture* browseTex;
	ofTexture* browseActiveTex;
	ofTexture* browseDeactivatedTex;
	ofTexture* postTex;
	ofTexture* postActiveTex;
	ofTexture* postDeactivatedTex;

	//Button dimensions for UI grow animation
	float postBtnXPos;
	float browserAddBtnXPos;
	float shuffleBtnXPos;
	float addDesignBtnXPos;
	ofVec2f postBtnSize;
	ofVec2f browseBtnSize;
	ofVec2f browserAddBtnSize;
	ofVec2f shuffleBtnSize;
	ofVec2f addDesignBtnSize;
};
