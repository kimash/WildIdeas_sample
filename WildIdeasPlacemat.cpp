//
//  WildIdeasPlacemat.cpp
//  
//
//  Created by Kim Ash on 4/29/15.
//
//

#include "WildIdeasPlacemat.h"
#include "GComponent.h"
#include "ImageChanger.h"
#include "Canvas.h"
#include "MoveableButtonPanel.h"
#include "ofxSuperLog.h"
#include "BrowserPanel.h"
#include "SketchBrowser.h"
#include "AppSpecificDefines.h"
#include "ofxTimeMeasurements.h"
#include "SvgButton.h"
#include "ofxRemoteUIServer.h"
#include "SvgSketchViewer.h"
#include "WIAppParams.h"
#include "ReaderController.h"
#include "WIGlobal.h"
#include "DialogBox.h"
#include "PulsateButton.h"
#include "TimeoutDialog.h"

void Placemat::setup(float x_, float y_, float w_, float h_)
{
	x = x_;
	y = y_;
	w = w_;
	h = h_;

	GComponent::setup(x, y, w, h);

	ImagePairPlaylist *pl = &(ImagePairPlaylist::one());	//for debugging

	if(ImagePairPlaylist::one().getFeaturedPair(1) != NULL){
		imagePair = ImagePairPlaylist::one().getFeaturedPair(1);
	} else {
		LOG << "No image pairs! Exiting!";
		ofExit();
	}

	//image changer
    imageChanger = new ImageChanger();
	imageChanger->setup(ImagePairPlaylist::one().getFeaturedLeftObjAt(0), 
		ImagePairPlaylist::one().getFeaturedRightObjAt(0), imagePair->leftObject, imagePair->rightObject, 
		(w-IMG_CHANGER_BOX_SIDE)/2, (h-IMG_CHANGER_BOX_SIDE)/2);
	imageChanger->setPlane(150);
	imageChanger->setName("Image Changer");
	addChild(imageChanger);
	playlistIndex = 1;

	//sketch viewer for attract loop
	sketchViewer = new SketchViewer();
	sketchViewer->setup();
	sketchViewer->setName("Sketch Viewer");
	sketchViewer->setPlane(170);
	addChild(sketchViewer);
	if(!imagePair->svgStrings.empty()){	 
		sketchViewer->setSvg(imagePair->svgStrings[0]);
	}
	svgShown = false;
	svgCount = 0;

	//canvas config and setup
	int index = 0;
	bool drawSelf = true;
	ofColor backgroundColor = (38);
	int undoBufferSize = 10;
	bool mipmappedFBO = true;

	mCanvas = new Canvas();
    mCanvas->setup( (w-CANVAS_W)/2, CANVAS_Y_POS, CANVAS_W, CANVAS_H, index, drawSelf, 
		backgroundColor, undoBufferSize, mipmappedFBO );
	mCanvas->setPlane(100);
	mCanvas->setName("Canvas");
	addChild(mCanvas);
	hasSketchToPost = false;
	
	lastPostNumPaths = 0;
	finalPathCubics = 0;
	finalPathLines = 0;
	finalPathStartPt.set(0,0);

	canvasBoundary = new Boundary();
	canvasBoundary->setup((w-CANVAS_W)/2, CANVAS_Y_POS, CANVAS_W, CANVAS_H);
	canvasBoundary->setPlane(2);
	addChild(canvasBoundary);

	canvasOverlay = new CanvasOverlay();
	canvasOverlay->setup((w-CANVAS_W)/2, CANVAS_Y_POS);
	canvasOverlay->setPlane(110);
	addChild(canvasOverlay);

	buttonPanel = new MoveableButtonPanel;
	buttonPanel->setup(BTN_PANEL_X_POS, BUTTON_Y_POS, mCanvas);
	buttonPanel->setPlane(170);
	buttonPanel->setName("Button Panel");
	addChild(buttonPanel);

	canvasCoachmark = new ofTexture();
	ofLoadImage(*canvasCoachmark, "coachmarks/coachmark_sketch.png");

	//dialog box and overlay
	dialogBox = new CanvasDialogBox();
	addChild(dialogBox);
	dialogBox->setup((w-DIALOG_BOX_W)/2, (h-DIALOG_BOX_H)/2);
	dialogBox->setName("Dialog Box");
	dialogBox->setPlane(200);

	overlayForDB = new DialogBoxOverlay();
	addChild(overlayForDB);
	overlayForDB->setup(0, 0);
	overlayForDB->setPlane(180);
	
	//headers
	header = new WildIdeasHeader();
	header->setup((w-CANVAS_W)/2, h/55);
	header->setPlane(2);
	header->setName("Header");
	addChild(header);

	categoryHeader = new AttractLoopHeader();
	categoryHeader->setup((w-CANVAS_W)/2, 600);
	categoryHeader->setPlane(2);
	categoryHeader->setName("Categories");
	addChild(categoryHeader);

	//browser and panel
	browser = new SketchBrowser();
	browser->setup(imagePair, 0, (h-CANVAS_H)/2);
	browser->setPlane(100);
	browser->setName("Browser");
	addChild(browser);

	browserPanel = new BrowserPanel();
	addChild(browserPanel);
	browserPanel->setup(imagePair, (w-CANVAS_W)/2, 3122);
	browserPanel->setPlane(2);
	browserPanel->setName("Browser Panel");

	//timeout dialog box
	timeoutDialog = new TimeoutDialog();
	addChild(timeoutDialog);
	timeoutDialog->setup((w-TIMEOUT_DIALOG_W)/2, (h-TIMEOUT_DIALOG_H)/2, overlayForDB);
	timeoutDialog->setPlane(200);

	//buttons:
	//Add Design button - now titled Start Designing
	addDesignButton = new PulsateButton();
	addYourDesignTex = new ofTexture();
	ofLoadImage(*addYourDesignTex, "UI/btn_add_home_default.png");
	addYourDesignActiveTex = new ofTexture();
	ofLoadImage(*addYourDesignActiveTex, "UI/btn_add_home_pressed.png");
	addDesignBtnXPos = (w-addYourDesignTex->getWidth())/2;
	addDesignBtnSize = ofVec2f(addYourDesignTex->getWidth(), addYourDesignTex->getHeight());
	addDesignButton->setup(addYourDesignTex, addYourDesignActiveTex, addDesignBtnXPos, 
		ADD_BTN_Y_POS, 0, 0);
	addDesignButton->setPlane(160);
	addDesignButton->setName("Add Design btn");
	addChild(addDesignButton);

	browserAddDesignButton = new ActiveTextureButton();
	addYourDesignAltTex = new ofTexture();
	ofLoadImage(*addYourDesignAltTex, "UI/btn_add_default.png");
	browserAddBtnSize = ofVec2f(addYourDesignAltTex->getWidth(), addYourDesignAltTex->getHeight());
	browserAddBtnXPos = w - addYourDesignAltTex->getWidth();
	browserAddDesignButton->setup(addYourDesignAltTex, addYourDesignActiveTex, browserAddBtnXPos, 
		BUTTON_Y_POS, 0, 0);
	browserAddDesignButton->setPlane(160);
	addChild(browserAddDesignButton);

	//alternate textures for browserAddDesignButton
	backToDesignTex = new ofTexture();
	ofLoadImage(*backToDesignTex, "UI/btn_backtodesign_default.png");

	backToDesignActiveTex = new ofTexture();
	ofLoadImage(*backToDesignActiveTex, "UI/btn_backtodesign_pressed.png");

	//Shuffle button
	shuffleButton = new PulsateButton();
	shuffleTex = new ofTexture();
	ofLoadImage(*shuffleTex, "UI/btn_shuffle_default.png");
	shuffleActiveTex = new ofTexture();
	ofLoadImage(*shuffleActiveTex, "UI/btn_shuffle_pressed.png");
	shuffleBtnXPos = (w-shuffleTex->getWidth())/2;
	shuffleBtnSize = ofVec2f(shuffleTex->getWidth(), shuffleTex->getHeight());
	shuffleButton->setup(shuffleTex, shuffleActiveTex, shuffleBtnXPos, SHUFFLE_BTN_Y_POS, 0, 0);
	shuffleButton->setPlane(160);
	shuffleButton->setName("Shuffle btn");
	addChild(shuffleButton);

	//Post button
	postButton = new ActiveTextureButton();
	postTex = new ofTexture();
	ofLoadImage(*postTex, "UI/btn_post_default.png");
	postActiveTex = new ofTexture();
	ofLoadImage(*postActiveTex, "UI/btn_post_pressed.png");
	postBtnSize = ofVec2f(postTex->getWidth(), postTex->getHeight());
	postBtnXPos = w-postTex->getWidth();
	postButton->setup(postTex, postActiveTex, postBtnXPos, BUTTON_Y_POS, 0, 0);
	postButton->setPlane(160);
	postButton->setName("Post btn");
	addChild(postButton);

	//alternate texture for post button
	postDeactivatedTex = new ofTexture();
	ofLoadImage(*postDeactivatedTex, "UI/btn_post_deactive.png");

	//Browse button
	browseButton = new ActiveTextureButton();
	browseTex = new ofTexture();
	ofLoadImage(*browseTex, "UI/btn_browse_default.png");
	browseActiveTex = new ofTexture();
	ofLoadImage(*browseActiveTex, "UI/btn_browse_pressed.png");
	browseBtnSize = ofVec2f(browseTex->getWidth(), browseTex->getHeight());
	browseButton->setup(browseTex, browseActiveTex, 0, BUTTON_Y_POS, 0, 0);;
	browseButton->setPlane(160);
	browseButton->setName("Browse btn");
	addChild(browseButton);

	//alternate texture for browse button
	browseDeactivatedTex = new ofTexture();
	ofLoadImage(*browseDeactivatedTex, "UI/btn_browse_deactive.png");
	
	//touch listeners
	ofAddListener(addDesignButton->eventClick, this, &Placemat::onAddDesignBtnTouch);
	ofAddListener(browserAddDesignButton->eventClick, this, &Placemat::onBrowserAddDesignBtnTouch);
	ofAddListener(postButton->eventClick, this, &Placemat::onPostBtnTouch);
	ofAddListener(browseButton->eventClick, this, &Placemat::onBrowseBtnTouch);
	ofAddListener(shuffleButton->eventClick, this, &Placemat::onShuffleBtnTouch);
	ofAddListener(browserPanel->eventTouchDown, this, &Placemat::onBrowserPanelTouch);
	ofAddListener(sketchViewer->eventTouchDown, this, &Placemat::onSketchViewerTouch);
	ofAddListener(imageChanger->eventTouchDown, this, &Placemat::onImageChangerTouch);

	//animation listeners
	ofAddListener(imageChanger->slideDownAnim.animFinished, this, &Placemat::onImageSlideDownDone);
	ofAddListener(browserPanel->growLineAnim.animFinished, this, &Placemat::onBPTransitionDone);
	ofAddListener(imageChanger->togetherAnim.animFinished, this, &Placemat::onTogetherAnimDone);
	ofAddListener(header->titleFadeIn.animFinished, this, &Placemat::onHeaderAnimDone);
	ofAddListener(imageChanger->rotateAnim.animFinished, this, &Placemat::onRotateAnimDone);
	ofAddListener(sketchViewerGrowAnim.animFinished, this, &Placemat::onSketchViewerGrowAnimDone);
	ofAddListener(sketchViewerShrinkAnim.animFinished, this, &Placemat::onSketchViewerShrinkAnimDone);
	ofAddListener(buttonGrowAnim.animFinished, this, &Placemat::onButtonGrowAnimDone);
	ofAddListener(buttonShrinkAnim.animFinished, this, &Placemat::onShrinkButtonAnimDone);
	ofAddListener(dialogBox->timer.animFinished, this, &Placemat::onDialogBoxTimeoutDone);
	ofAddListener(timeoutDialog->timer.animFinished, this, &Placemat::onPlacematTimeoutDone);

	prevState = BROWSE;		//just a default value
	nextState = DRAWING;	//ditto
	setState(ATTRACT);
}

Placemat::~Placemat()
{
	delete canvasCoachmark;
	delete addYourDesignTex;
	delete addYourDesignAltTex;
	delete addYourDesignActiveTex;
	delete backToDesignTex;
	delete backToDesignActiveTex;
	delete shuffleTex;
	delete shuffleActiveTex;
	delete postTex;
	delete postActiveTex;
	delete postDeactivatedTex;
	delete browseTex;
	delete browseActiveTex;
	delete browseDeactivatedTex;

	//touch listeners
	ofRemoveListener(addDesignButton->eventClick, this, &Placemat::onAddDesignBtnTouch);
	ofRemoveListener(browserAddDesignButton->eventClick, this, &Placemat::onBrowserAddDesignBtnTouch);
	ofRemoveListener(postButton->eventClick, this, &Placemat::onPostBtnTouch);
	ofRemoveListener(browseButton->eventClick, this, &Placemat::onBrowseBtnTouch);
	ofRemoveListener(shuffleButton->eventClick, this, &Placemat::onShuffleBtnTouch);
	ofRemoveListener(browserPanel->eventTouchDown, this, &Placemat::onBrowserPanelTouch);
	ofRemoveListener(sketchViewer->eventTouchDown, this, &Placemat::onSketchViewerTouch);
	ofRemoveListener(imageChanger->eventTouchDown, this, &Placemat::onImageChangerTouch);
	ofRemoveListener(dialogBox->timer.animFinished, this, &Placemat::onDialogBoxTimeoutDone);
	ofRemoveListener(timeoutDialog->timer.animFinished, this, &Placemat::onPlacematTimeoutDone);

	//animation listeners
	ofRemoveListener(imageChanger->slideDownAnim.animFinished, this, &Placemat::onImageSlideDownDone);
	ofRemoveListener(browserPanel->growLineAnim.animFinished, this, &Placemat::onBPTransitionDone);
	ofRemoveListener(imageChanger->togetherAnim.animFinished, this, &Placemat::onTogetherAnimDone);
	ofRemoveListener(header->titleFadeIn.animFinished, this, &Placemat::onHeaderAnimDone);
	ofRemoveListener(imageChanger->rotateAnim.animFinished, this, &Placemat::onRotateAnimDone);
	ofRemoveListener(sketchViewerGrowAnim.animFinished, this, &Placemat::onSketchViewerGrowAnimDone);
	ofRemoveListener(sketchViewerShrinkAnim.animFinished, this, &Placemat::onSketchViewerShrinkAnimDone);
	ofRemoveListener(buttonGrowAnim.animFinished, this, &Placemat::onButtonGrowAnimDone);
	ofRemoveListener(buttonShrinkAnim.animFinished, this, &Placemat::onShrinkButtonAnimDone);

	LOG << "destroying placemat";
}

void Placemat::setState(State s)
{
	state = s;

	switch(state) 
	{
	case ATTRACT:
		LOG << "Set state ATTRACT";
		mCanvas->deactivate();
		canvasOverlay->deactivate();
		buttonPanel->deactivate();
		postButton->deactivate();
		browseButton->deactivate();
		canvasBoundary->deactivate();
		browser->deactivate();
		browserAddDesignButton->deactivate();
		browserAddDesignButton->changeTextures(addYourDesignAltTex, addYourDesignActiveTex);
		dialogBox->deactivate();
		overlayForDB->deactivate();
		timeoutDialog->deactivate();
		timeoutDialog->enableTimer(false);
		imageChanger->activate();
		addDesignButton->activate();
		categoryHeader->activate();
		shuffleButton->activate();
		header->setState(WildIdeasHeader::ATTRACT);
		categoryHeader->setState(AttractLoopHeader::DISPLAY);
		if(imageChanger->getState() == ImageChanger::AWAKE){
			imageChanger->setState(ImageChanger::AWAKE);
		} else if(imageChanger->getState() == ImageChanger::SHUFFLE 
			|| imageChanger->getState() == ImageChanger::REVERSE_TRANSITION){
				imageChanger->setState(ImageChanger::AWAKE);
                shuffleButton->setState(PulsateButton::PULSATE);
				addDesignButton->setState(PulsateButton::NORMAL);
		} else if (imageChanger->getState() == ImageChanger::SPLIT){
			if(imageChanger->isTimeoutDone){
				imageChanger->setState(ImageChanger::ATTRACT);
			} else {
				imageChanger->setState(ImageChanger::AWAKE);
                shuffleButton->setState(PulsateButton::PULSATE);
				addDesignButton->setState(PulsateButton::NORMAL);
			}
		} else {
			imageChanger->setState(ImageChanger::ATTRACT);
		}
		if(imagePair->svgStrings.empty()){
			browserPanel->setState(BrowserPanel::NO_SKETCHES);
		} else {
			browserPanel->setState(BrowserPanel::SHOW_SKETCHES);
		}
		break;
	case BROWSE:
		LOG << "Set state BROWSE";
		header->setState(WildIdeasHeader::BROWSE);
		browserPanel->setState(BrowserPanel::SHOW_IMAGE_PAIR);
		mCanvas->deactivate();
		canvasOverlay->deactivate();
		buttonPanel->deactivate();
		postButton->deactivate();
		browseButton->deactivate();
		canvasBoundary->deactivate();
		categoryHeader->deactivate();
		shuffleButton->deactivate();
		imageChanger->deactivate();
		addDesignButton->deactivate();
		dialogBox->deactivate();
		overlayForDB->deactivate();
		timeoutDialog->enableTimer(true);
		timeoutDialog->startTimer();
		browser->activate();
		browser->setState(SketchBrowser::DISPLAY);
		browserAddDesignButton->activate();
		break;
	case TRANSITION:
		LOG << "Set state TRANSITION";
		startAttractBtnShrinkAnim();
		dialogBox->deactivate();
		overlayForDB->deactivate();
		timeoutDialog->deactivate();
		timeoutDialog->enableTimer(false);
		canvasOverlay->deactivate();
		buttonGrowAnim.reset(0.0);
		imageChanger->setState(ImageChanger::TRANSITION);
		browserPanel->setState(BrowserPanel::TITLE_FADE_OUT);
		imageChanger->setEnabled(false);
		break;
	case DRAWING:
		LOG << "Set state DRAWING";
		header->setState(WildIdeasHeader::DRAWING);
		browserPanel->setState(BrowserPanel::SHOW_IMAGE_PAIR);
		imageChanger->deactivate();
		shuffleButton->deactivate();
		categoryHeader->deactivate();
		addDesignButton->deactivate();
		browserAddDesignButton->deactivate();
		browser->deactivate();
		dialogBox->deactivate();
		timeoutDialog->enableTimer(true);
		timeoutDialog->startTimer();
		overlayForDB->deactivate();
		canvasOverlay->deactivate();
		mCanvas->activate();
		buttonPanel->activate();
		canvasBoundary->activate();
		postButton->activate();
		browseButton->activate();
		buttonPanel->setPosition(ofVec2f(w-770, BUTTON_Y_POS));
		if(prevState != BROWSE){
			mCanvas->clear();
		} else if(prevState == BROWSE && !hasSketchToPost){ //after viewing posted sketch
			mCanvas->clear();
		}
		if(!hasSketchToPost){
			startCanvasCoachmarkAnim();
		}
		break;
	case REVERSE_TRANSITION:
		LOG << "Set state REVERSE_TRANSITION";
		mCanvas->deactivate();
		buttonPanel->deactivate();
		postButton->deactivate();
		browseButton->deactivate();
		canvasBoundary->deactivate();
		canvasOverlay->deactivate();
		timeoutDialog->deactivate();
		timeoutDialog->enableTimer(false);
		browser->deactivate();
		browserAddDesignButton->deactivate();
		dialogBox->deactivate();
		overlayForDB->deactivate();
		imageChanger->activate();
		categoryHeader->activate();
		browserPanel->setState(BrowserPanel::TITLE_FADE_OUT);
		imageChanger->setState(ImageChanger::REVERSE_TRANSITION);
		imageChanger->setEnabled(false);
		break;
	case SHRINK_UI:
		LOG << "Set state SHRINK_UI";
		timeoutDialog->deactivate();
		timeoutDialog->enableTimer(false);
		header->setState(WildIdeasHeader::FADE_OUT);
		startShrinkButtonAnim();
		if(prevState == BROWSE){
			browser->setState(SketchBrowser::SHRINK_UI);
		} else if(prevState == DRAWING){
			canvasBoundary->hide();
			canvasOverlay->show();
		}
		break;
	case GROW_UI:
		LOG << "Set state GROW_UI";
		timeoutDialog->deactivate();
		timeoutDialog->enableTimer(false);
		startButtonGrowAnim();
		if(nextState == BROWSE){
			browser->activate();
			browser->setState(SketchBrowser::GROW_UI);
			header->setState(WildIdeasHeader::FADE_IN);
		} else if(nextState == DRAWING){
			canvasBoundary->show();
			canvasOverlay->hide();
			header->setState(WildIdeasHeader::FADE_IN);
		}
		break;
	default:
		LOG << "No placemat state? Something went wrong.";
		break;
	}
}

void Placemat::updateState()
{
	switch(state) {
	case ATTRACT:
		imageChanger->update();
        browserPanel->update();
        shuffleButton->update();
        addDesignButton->update();
		if(sketchViewerGrowAnim.isOrWillBeAnimating() || sketchViewerShrinkAnim.isOrWillBeAnimating()){
			addDesignButton->setEnabled(false);
			shuffleButton->setEnabled(false);
		} else {
            addDesignButton->setEnabled(true);
            shuffleButton->setEnabled(true);
        }
        if(svgShown){
			sketchViewerShrinkAnim.update(DT);
        }else{
			sketchViewerGrowAnim.update(DT);
        }
		break;
	case BROWSE:
		browserPanel->update();
		timeoutDialog->update();
		break;
	case TRANSITION:
		imageChanger->update();
		browserPanel->update();
		header->update();
		categoryHeader->update();
		buttonGrowAnim.update(DT);
		attractBtnShrinkAnim.update(DT); //only for shuffle and addDesign
		canvasBoundary->update(DT);
		if(nextState == BROWSE){
			browser->update();
		}
		break;
	case DRAWING:
		canvasCoachmarkAnim.update(DT);
		dialogBox->updateTimer();
		timeoutDialog->update();
		break;
	case REVERSE_TRANSITION:
		header->update();
		categoryHeader->update();
		imageChanger->update();
		browserPanel->update();
		break;
	case SHRINK_UI:
		buttonShrinkAnim.update(DT);
		canvasBoundary->update(DT);
		canvasOverlay->update(DT);
		header->update();
		browserPanel->update();
		if(prevState == BROWSE){
			browser->update();
		}
		break;
	case GROW_UI:
		buttonGrowAnim.update(DT);
		canvasBoundary->update(DT);
		canvasOverlay->update(DT);
		header->update();
		browserPanel->update();
		if(nextState == BROWSE){
			browser->update();
		}
		break;
	default:
		break;
	}
}

void Placemat::draw()
{
	switch (state)
	{
	case ATTRACT:
		if(svgShown){
			drawSketchViewerShrinkAnim();
		} else {
			drawSketchViewerGrowAnim();
		}
		break;
	case TRANSITION:
		drawButtonGrowAnim();
		drawAttractBtnShrinkAnim();
		break;
	case BROWSE:
		break;
	case DRAWING:
		{	
		drawCanvasCoachmarkAnim();
		//disable posting designs until there are enough points to make a drawing
		if(mCanvas->pathHistory().size() > 0){
			//check that lines are drawn, not just moveTos
			bool linesDrawn = false;
			for(int i = 0; i < mCanvas->pathHistory().size(); i++){
				pathLayer p = mCanvas->pathHistory()[i];
				if(p.cubicPaths.size() > 0 || p.lines.size() > 0){
					linesDrawn = true;
				}
			}//prevent app from posting blank sketches
			if(linesDrawn){
				hasSketchToPost = true;
				postButton->changeTextures(postTex, postActiveTex);
			} else {
				hasSketchToPost = false;
				postButton->changeTextures(postDeactivatedTex, postDeactivatedTex);
			}
		} else {
			hasSketchToPost = false;
			postButton->changeTextures(postDeactivatedTex, postDeactivatedTex);
		}
		break;
		}
	case SHRINK_UI:
		drawShrinkButtonAnim();
		break;
	case GROW_UI:
		drawButtonGrowAnim();
		break;
	default:
		break;
	}
}

void Placemat::setNewPair(ImagePairPlaylist::ImagePair* imagePair_)
{
	imagePair = imagePair_;
	imageChanger->setNewPair(imagePair->leftObject, imagePair->rightObject);
	browserPanel->setNewPair(imagePair);
	browser->setNewPair(imagePair);
	if(!imagePair->svgStrings.empty()){
		sketchViewer->setSvg(imagePair->svgStrings[0]);
	} else {
		sketchViewer->setSvgFromPath("coachmarks/coach_youridea.svg");
		svgCount = 1;
	}
	LOG << "loaded image pair " << ImagePairPlaylist::one().getLeftObject(imagePair)->title <<
		"/" << ImagePairPlaylist::one().getRightObject(imagePair)->title;
}

void Placemat::advancePlaylist()
{
	TS_START("advancePlaylist");
	playlistIndex++;
	if (playlistIndex >= ImagePairPlaylist::one().getFeaturedPlaylistSize()) {
		playlistIndex = 0;
	}
	setNewPair(ImagePairPlaylist::one().getFeaturedPair(playlistIndex));
	LOG << "Advance playlist to pair #" << playlistIndex;
	TS_STOP("advancePlaylist");
}

void Placemat::showMyCreationInBrowser()
{
	prevState = DRAWING;
	nextState = BROWSE;
	browser->showSketchAt(imagePair->svgStrings.size()-1);
	hasSketchToPost = false;
	setState(SHRINK_UI);
}

void Placemat::resetTimeoutTimer()
{
	//need to disable timeout while pen popup is visible
	timeoutDialog->enableTimer(false);
}


//UI interaction
void Placemat::onAddDesignBtnTouch(TouchEvent& event)
{
	nextState = DRAWING;
	header->setNextState(WildIdeasHeader::DRAWING);
	setState(TRANSITION);
}

void Placemat::onBrowserAddDesignBtnTouch(TouchEvent& event)
{
	prevState = BROWSE;
	nextState = DRAWING;
	header->setNextState(WildIdeasHeader::DRAWING);
	setState(SHRINK_UI);
}

void Placemat::onShuffleBtnTouch(TouchEvent& event)
{
	if(state == ATTRACT) {
		imageChanger->setState(ImageChanger::SHUFFLE);
		setNewPair(ImagePairPlaylist::one().getRandomPair());
	}
	LOG << "Shuffle";
}

void Placemat::onBrowseBtnTouch(TouchEvent& event)
{
	if(!imagePair->svgStrings.empty()){
		browserAddDesignButton->changeTextures(backToDesignTex, backToDesignActiveTex);
		prevState = DRAWING;
		nextState = BROWSE;
		header->setNextState(WildIdeasHeader::BROWSE);
		setState(SHRINK_UI);
	}
}

void Placemat::onPostBtnTouch(TouchEvent& event)
{
	if(hasSketchToPost) {
		bool okToPost = false;
		//if there are fewer paths than the last post, we know it's a different sketch
		//if there was no previous post, lastPostNumPaths == 0 and it's ok since it's the first one 
		if(lastPostNumPaths > mCanvas->pathHistory().size() || lastPostNumPaths == 0){
			okToPost = true;
			//if same number of paths, need to check that they're different
			//compare final paths
		} else if(lastPostNumPaths == mCanvas->pathHistory().size()){
			if(finalPathCubics != mCanvas->pathHistory().back().cubicPaths.size()){
				okToPost = true;
			}
			if(finalPathLines != mCanvas->pathHistory().back().lines.size()){
				okToPost = true;
			}
			if(finalPathStartPt != mCanvas->pathHistory().back().startPoint){
				okToPost = true;
			}
		} else {
			//if more paths, need to check that additional paths are not just moveTos
			for(int i = lastPostNumPaths; i < mCanvas->pathHistory().size(); i++){
				pathLayer p = mCanvas->pathHistory()[i];
				if(p.cubicPaths.size() > 0 || p.lines.size() > 0){
					okToPost = true;
				} 
			}
		}

		if(okToPost){
			string filename = "wildIdeas" + ofGetTimestampString();
			string filepath = "SVG/" + filename + ".svg";
			mCanvas->saveToFile(filepath);
			LOG << filename << " saved to disk!";

			lastPostNumPaths = mCanvas->pathHistory().size();
			finalPathCubics = mCanvas->pathHistory().back().cubicPaths.size();
			finalPathLines = mCanvas->pathHistory().back().lines.size();
			finalPathStartPt = mCanvas->pathHistory().back().startPoint;

			//add this SVG to this ImagePair's collection
			ImagePairPlaylist::one().addSvgToImagePair(imagePair, ofBufferFromFile(filepath));
		
			//update browser panel sketch buttons to reflect new sketch upon return to ATTRACT
			browserPanel->setSketchButtonData();

			//post to CMS
			WIGlobal::postToCMS(filepath, imagePair->leftObject->id, imagePair->leftObject->category,
				imagePair->rightObject->id, imagePair->rightObject->category, AppParams::allSketchesFeatured);

			//save to pen
			if(AppParams::bPenFeatures){
				ofPixels pix;
				mCanvas->getTexture()->readToPixels(pix);
				if(ReaderController::one().isReaderReady(this)){
					ReaderController::one().promtUserToHoldPenToSaveCreation(this, pix, "wild-ideas",
						"");
				}
			} else {
				showMyCreationInBrowser();
			}
		} else {
			LOG << "That sketch has already been posted! Can't post it again.";
		}
	}
}

void Placemat::onBrowserPanelTouch(TouchEvent& event)
{
	prevState = state;
	switch (state)
	{
	case Placemat::ATTRACT:
		break;
	case Placemat::BROWSE:
		mCanvas->clear();	//always clear canvas on back to start
		hasSketchToPost = false;
		setState(SHRINK_UI);
		break;
	case Placemat::DRAWING:
		if(hasSketchToPost){
			overlayForDB->activate();
			dialogBox->activate();
			dialogBox->startTimer();
			timeoutDialog->enableTimer(false);	//both dialog boxes can't be onscreen at same time
		} else {
			mCanvas->clear();	//always clear canvas on back to start
			hasSketchToPost = false;
			setState(SHRINK_UI);
		}
		break;
	default:
		break;
	}
}

void Placemat::onSketchBtnClick(int& sketchIndex)
{
	//interrupt sketch viewer animations if necessary
	if(sketchViewerGrowAnim.isOrWillBeAnimating()){
		sketchViewer->deactivate();
		sketchViewerGrowAnim.reset(0.0);
		svgShown = false;
	} else if(sketchViewerShrinkAnim.isOrWillBeAnimating()){
		sketchViewer->deactivate();
		sketchViewerShrinkAnim.reset(1.0);
		svgShown = false;
	}
	
	//show associated sketch in browser after clicking sketch button
	browser->showSketchAt(sketchIndex);
	nextState = BROWSE;
	header->setNextState(WildIdeasHeader::BROWSE);
	setState(TRANSITION);
}

void Placemat::onDBoxStartOverClick(TouchEvent& event)
{
	dialogBox->deactivate();
	overlayForDB->deactivate();
	timeoutDialog->enableTimer(true);	//timer is disabled when dialog box is up
	timeoutDialog->startTimer();
	hasSketchToPost = false;
	prevState = DRAWING;
	setState(SHRINK_UI);
}

void Placemat::onDBoxBackToDesignClick(TouchEvent& event)
{
	dialogBox->deactivate();
	overlayForDB->deactivate();
	timeoutDialog->enableTimer(true);	//timer is disabled when dialog box is up
	timeoutDialog->startTimer();
}

void Placemat::onSketchViewerTouch(TouchEvent& event)
{
	if(sketchViewerGrowAnim.isOrWillBeAnimating()){
		sketchViewer->deactivate();
		sketchViewerGrowAnim.reset(0.0);
		svgShown = false;
	}else if(sketchViewerShrinkAnim.isOrWillBeAnimating()){
		sketchViewer->deactivate();
		sketchViewerShrinkAnim.reset(1.0);
		svgShown = false;
	}
	shuffleButton->activate();
	addDesignButton->activate();
	imageChanger->setState(ImageChanger::AWAKE);
    shuffleButton->setState(PulsateButton::PULSATE);
    addDesignButton->setState(PulsateButton::NORMAL);
}

void Placemat::onImageChangerTouch(TouchEvent& event)
{
	if(sketchViewerGrowAnim.isOrWillBeAnimating()){
		sketchViewer->deactivate();
		sketchViewerGrowAnim.reset(0.0);
		svgShown = false;
		imageChanger->setState(ImageChanger::AWAKE);
		shuffleButton->setState(PulsateButton::PULSATE);
		addDesignButton->setState(PulsateButton::NORMAL);
	} else if(sketchViewerShrinkAnim.isOrWillBeAnimating()){
		sketchViewer->deactivate();
		sketchViewerShrinkAnim.reset(1.0);
		svgShown = false;
		imageChanger->setState(ImageChanger::AWAKE);
		shuffleButton->setState(PulsateButton::PULSATE);
		addDesignButton->setState(PulsateButton::NORMAL);
	} else {
		imageChanger->isTouched = true;
		imageChanger->isTimeoutDone = false;

		if(imageChanger->getState() != ImageChanger::SPLIT){
			imageChanger->setState(ImageChanger::SPLIT);
			addDesignButton->setState(PulsateButton::PULSATE);
			shuffleButton->setState(PulsateButton::NORMAL);
		} else {
			imageChanger->startTogetherAnim();
		}
	}
}

//animation callbacks
void Placemat::onRotateAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	sketchViewer->activate();
	startSketchViewerGrowAnim();
    addDesignButton->setState(PulsateButton::FADE_OUT);
    shuffleButton->setState(PulsateButton::FADE_OUT);
}

void Placemat::onImageSlideDownDone(ofxAnimatable::AnimationEvent& animEvent)
{
	header->setState(WildIdeasHeader::FADE_OUT);
	categoryHeader->setState(AttractLoopHeader::FADE_OUT);
	browserPanel->setState(BrowserPanel::TRANSITION);
	browserPanel->setNextState(BrowserPanel::SHOW_IMAGE_PAIR);
	browserPanel->startTransitionAnim();
}

void Placemat::onBPTransitionDone(ofxAnimatable::AnimationEvent& animEvent)
{	
	if(nextState == BROWSE){
		startButtonGrowAnim();
		browser->activate();
		browser->setState(SketchBrowser::GROW_UI);
		header->setNextState(WildIdeasHeader::BROWSE);
		header->setState(WildIdeasHeader::FADE_IN);
	} else if(nextState == DRAWING){
		startButtonGrowAnim();
		canvasBoundary->show();
		canvasOverlay->hide();
		header->setState(WildIdeasHeader::FADE_IN);
	}
	
}

void Placemat::onTogetherAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	imageChanger->isTouched = false;
	imageChanger->isSeparateAnimDone = false;
	
	if(state == REVERSE_TRANSITION){
		header->setNextState(WildIdeasHeader::ATTRACT);
		header->setState(WildIdeasHeader::FADE_IN);
		categoryHeader->setState(AttractLoopHeader::FADE_IN);
		if(imagePair->svgStrings.empty()){
			browserPanel->setNextState(BrowserPanel::NO_SKETCHES);
		} else {
			browserPanel->setNextState(BrowserPanel::SHOW_SKETCHES);
		}
		nextState = ATTRACT;
		browserPanel->startTransitionAnim();
	} else {
		imageChanger->isSlideUpDone = false;
		setState(ATTRACT);
	}
}

void Placemat::onHeaderAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	if(state == REVERSE_TRANSITION){
		nextState = ATTRACT;
		setState(GROW_UI);
	}
}

void Placemat::onSketchViewerGrowAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	svgShown = true;
	svgCount++;
	startSketchViewerShrinkAnim();
    if (svgCount == 2) {
        addDesignButton->setState(PulsateButton::FADE_IN);
        shuffleButton->setState(PulsateButton::FADE_IN);
    }
}

void Placemat::onSketchViewerShrinkAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	svgShown = false;
	if(svgCount < 2){
		sketchViewer->setSvgFromPath("coachmarks/coach_youridea.svg");
		startSketchViewerGrowAnim();
	} else {
		svgCount = 0;
		addDesignButton->activate();
		shuffleButton->activate();
        addDesignButton->setState(PulsateButton::NORMAL);
        shuffleButton->setState(PulsateButton::NORMAL);
		sketchViewer->deactivate();
		advancePlaylist();
		imageChanger->onRotateSeqDone();
	}
}

void Placemat::onButtonGrowAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	setState(nextState);
}

void Placemat::onShrinkButtonAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	if(prevState == DRAWING && nextState == BROWSE){
		browseButton->deactivate();
		buttonPanel->deactivate();
		postButton->deactivate();
		canvasBoundary->deactivate();
		setState(GROW_UI);
	} else if(prevState == BROWSE && nextState == DRAWING){
		browserAddDesignButton->deactivate();
		setState(GROW_UI);
	} else {
		setState(REVERSE_TRANSITION);
	}
}

void Placemat::onDialogBoxTimeoutDone(ofxAnimatable::AnimationEvent& animEvent)
{
	dialogBox->deactivate();
	overlayForDB->deactivate();
	timeoutDialog->enableTimer(true);	//timeout is disabled while dialog box is up
}

void Placemat::onPlacematTimeoutDone(ofxAnimatable::AnimationEvent& animEvent)
{
	timeoutDialog->deactivate();
	overlayForDB->deactivate();
	mCanvas->clear();
	hasSketchToPost = false;

	if(state == DRAWING){
		prevState = DRAWING;
	} else if(state == BROWSE) {
		prevState = BROWSE;
	}
	setState(SHRINK_UI);
}


void Placemat::startSketchViewerGrowAnim()
{
	LOG << "Grow SketchViewer";
	sketchViewerGrowAnim.reset(0.0);
	sketchViewerGrowAnim.setCurve(AppParams::sketchGrowCurve);
	sketchViewerGrowAnim.setDuration(AppParams::sketchGrowDuration);
	sketchViewerGrowAnim.animateTo(1.0);
}

void Placemat::drawSketchViewerGrowAnim()
{
	sketchViewer->setScale(sketchViewerGrowAnim.val());
	sketchViewer->setPosition(ofVec2f((w-sketchViewer->getScaledWidth())/2, 
		(h-sketchViewer->getScaledHeight())/2));
}

void Placemat::startSketchViewerShrinkAnim()
{
	LOG << "Shrink SketchViewer";
	sketchViewerShrinkAnim.reset(1.0);
	sketchViewerShrinkAnim.setCurve(AppParams::sketchShrinkCurve);
	sketchViewerShrinkAnim.setDuration(AppParams::sketchGrowDuration);
	sketchViewerShrinkAnim.animateToAfterDelay(0.0, AppParams::sketchDelay);
}

void Placemat::drawSketchViewerShrinkAnim()
{
	sketchViewer->setScale(sketchViewerShrinkAnim.val());
	sketchViewer->setPosition(ofVec2f((w-sketchViewer->getScaledWidth())/2, 
		(h-sketchViewer->getScaledHeight())/2));
}

void Placemat::startButtonGrowAnim()
{
	buttonGrowAnim.reset(0.0);
	buttonGrowAnim.setCurve(AppParams::growUICurve);
	buttonGrowAnim.setDuration(AppParams::uiAnimDuration);
	buttonGrowAnim.animateTo(1.0);
}

void Placemat::drawButtonGrowAnim()
{
	if(nextState == DRAWING){
		postButton->activate();
		postButton->changeTextures(postDeactivatedTex, postDeactivatedTex);
		browseButton->activate();
		if(imagePair->svgStrings.empty()){
			browseButton->changeTextures(browseDeactivatedTex, browseDeactivatedTex);
		} else{
			browseButton->changeTextures(browseTex, browseActiveTex);
		}
		buttonPanel->activate();
		canvasBoundary->activate();
		postButton->setScale(buttonGrowAnim.val());
		postButton->setPosition(ofVec2f(postBtnXPos + (postBtnSize.x-postButton->getScaledWidth())/2,
			BUTTON_Y_POS + (postBtnSize.y-postButton->getScaledWidth())/2));
		browseButton->setScale(buttonGrowAnim.val());
		browseButton->setPosition(ofVec2f((browseBtnSize.x-browseButton->getScaledWidth())/2, 
			BUTTON_Y_POS + (browseBtnSize.y-browseButton->getScaledHeight())/2));
		buttonPanel->setScale(buttonGrowAnim.val());
		buttonPanel->setPosition(ofVec2f(BTN_PANEL_X_POS + (buttonPanel->getPanelW() - 
			buttonPanel->getScaledWidth())/2, BUTTON_Y_POS + 
			(buttonPanel->getPanelH()-buttonPanel->getScaledHeight())/2));
	} else if(nextState == BROWSE){
		canvasBoundary->deactivate();
		browserAddDesignButton->activate();
		browserAddDesignButton->setScale(buttonGrowAnim.val());
		browserAddDesignButton->setPosition(ofVec2f(browserAddBtnXPos + 
			(browserAddBtnSize.x - browserAddDesignButton->getScaledWidth())/2, 
			BUTTON_Y_POS + (browserAddBtnSize.y-browserAddDesignButton->getScaledHeight())/2));
	} else if(nextState == ATTRACT){
		shuffleButton->activate();
		shuffleButton->setScale(buttonGrowAnim.val());
		shuffleButton->setPosition(ofVec2f(shuffleBtnXPos + 
			(shuffleBtnSize.x - shuffleButton->getScaledWidth())/2,
			SHUFFLE_BTN_Y_POS + (shuffleBtnSize.y - shuffleButton->getScaledHeight())/2));
		addDesignButton->activate();
		addDesignButton->setScale(buttonGrowAnim.val());
		addDesignButton->setPosition(ofVec2f(addDesignBtnXPos + 
			(addDesignBtnSize.x - addDesignButton->getScaledWidth())/2,
			ADD_BTN_Y_POS + (addDesignBtnSize.y - addDesignButton->getScaledHeight())/2));
	}

}

void Placemat::startShrinkButtonAnim()
{
	buttonShrinkAnim.reset(1.0);
	buttonShrinkAnim.setCurve(AppParams::shrinkUICurve);
	buttonShrinkAnim.setDuration(AppParams::uiAnimDuration);
	buttonShrinkAnim.animateTo(0.0);
}

void Placemat::drawShrinkButtonAnim()
{
	if(prevState == DRAWING){
		postButton->setScale(buttonShrinkAnim.val());
		postButton->setPosition(ofVec2f(postBtnXPos + (postBtnSize.x-postButton->getScaledWidth())/2,
			BUTTON_Y_POS + (postBtnSize.y-postButton->getScaledWidth())/2));
		browseButton->setScale(buttonShrinkAnim.val());
		browseButton->setPosition(ofVec2f((browseBtnSize.x-browseButton->getScaledWidth())/2, 
			BUTTON_Y_POS + (browseBtnSize.y-browseButton->getScaledHeight())/2));
		buttonPanel->setScale(buttonShrinkAnim.val());
		buttonPanel->setPosition(ofVec2f(BTN_PANEL_X_POS + (buttonPanel->getPanelW() - 
			buttonPanel->getScaledWidth())/2, BUTTON_Y_POS + 
			(buttonPanel->getPanelH()-buttonPanel->getScaledHeight())/2));
	} else if(prevState == BROWSE){
		browserAddDesignButton->setScale(buttonShrinkAnim.val());
		browserAddDesignButton->setPosition(ofVec2f(browserAddBtnXPos + 
			(browserAddBtnSize.x - browserAddDesignButton->getScaledWidth())/2, 
			BUTTON_Y_POS + (browserAddBtnSize.y-browserAddDesignButton->getScaledHeight())/2));
	}
}

void Placemat::startAttractBtnShrinkAnim()
{
	attractBtnShrinkAnim.reset(1.0);
	attractBtnShrinkAnim.setCurve(AppParams::shrinkUICurve);
	attractBtnShrinkAnim.setDuration(AppParams::uiAnimDuration);
	attractBtnShrinkAnim.animateTo(0.0);
}

void Placemat::drawAttractBtnShrinkAnim()
{
	shuffleButton->activate();
	shuffleButton->setScale(attractBtnShrinkAnim.val());
	shuffleButton->setPosition(ofVec2f(shuffleBtnXPos + 
		(shuffleBtnSize.x - shuffleButton->getScaledWidth())/2,
		SHUFFLE_BTN_Y_POS + (shuffleBtnSize.y - shuffleButton->getScaledHeight())/2));
	addDesignButton->activate();
	addDesignButton->setScale(attractBtnShrinkAnim.val());
	addDesignButton->setPosition(ofVec2f(addDesignBtnXPos + 
		(addDesignBtnSize.x - addDesignButton->getScaledWidth())/2,
		ADD_BTN_Y_POS + (addDesignBtnSize.y - addDesignButton->getScaledHeight())/2));
}

void Placemat::startCanvasCoachmarkAnim()
{
	canvasCoachmarkAnim.reset(0);
	canvasCoachmarkAnim.setCurve(TANH);
	canvasCoachmarkAnim.setDuration(1.0);
	canvasCoachmarkAnim.setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
	canvasCoachmarkAnim.animateTo(1.0);
}

void Placemat::drawCanvasCoachmarkAnim()
{
	ofPushStyle();
	ofSetColor(255, 255*canvasCoachmarkAnim.val());
	canvasCoachmark->draw((w - canvasCoachmark->getWidth())/2, (h-canvasCoachmark->getHeight())/2-100, 
		canvasCoachmark->getWidth(), canvasCoachmark->getHeight());
	ofPopStyle();
}
