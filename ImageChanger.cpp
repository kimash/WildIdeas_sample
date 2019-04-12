//
//  ImageChanger.cpp
//  WildIdeasAttractLoop
//
//  Created by Kim Ash on 4/16/15.
//
//

#include "ImageChanger.h"
#include "ofxTimeMeasurements.h"
#include "AppSpecificDefines.h"
#include "ofxRemoteUIServer.h"
#include "WIGlobal.h"
#include "WIAppParams.h"

void ImageChanger::setup(WIObject* objLeft1, WIObject* objRight1, WIObject* objLeft2, WIObject* objRight2, float x_, float y_)
{
    texLeft1 = objLeft1->image;
    texRight1 = objRight1->image;
    texLeft2 = objLeft2->image;
    texRight2 = objRight2->image;
    
    x = x_;
    y = y_;
    w = IMG_CHANGER_BOX_SIDE;
    h = IMG_CHANGER_BOX_SIDE;
    
    threshold = AppParams::threshold;

	displayFont = WIGlobal::getHeaderFont();
	leftTitleText = ofToUpper(objLeft2->title);
	rightTitleText = ofToUpper(objRight2->title);
	textSize = 96.;
	shadowAlpha = 30.;

	texPlus = new ofTexture();
	ofLoadImage(*texPlus, "UI/plus_home.png");
	
	isSeparateAnimDone = false;
	isEnterSeqDone = false;
	isAttractSeqDone = false;
	isTouched = false;
	isTimeoutDone = false;
	isSlideUpDone = false;
    
    ofFbo::Settings s;
    s.width = w;
    s.height = h;
    s.internalformat = GL_RGBA;
    s.numSamples = 0;
    container.allocate(s);

    GComponent::setup(x, y, w, h);

	ofFbo::Settings f;
	f.width = IMG_CHANGER_BOX_SIDE;
	f.height = textSize * 6.;
	f.internalformat = GL_RGBA;
	f.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
	f.numSamples = 0;
	f.useDepth = false;
	f.useStencil = false;

	fboBlur.blurOffset = 10; //how much blur
	fboBlur.blurPasses = 2;
	fboBlur.numBlurOverlays = 1;
	fboBlur.blurOverlayGain = 255;

	fboBlur.setup(f, false);
    
	ofAddListener(alphaFadeIn.animFinished, this, &ImageChanger::onEnterSeqDone);
	ofAddListener(separateAnim.animFinished, this, &ImageChanger::onSeparateAnimDone);
	ofAddListener(slideUpAnim.animFinished, this, &ImageChanger::onSlideUpDone);
	ofAddListener(timeout.animFinished, this, &ImageChanger::onTimeoutDone);

	setState(ATTRACT);
}

ImageChanger::~ImageChanger()
{
	delete texPlus;
}

void ImageChanger::setNewPair(WIObject* newObjL, WIObject* newObjR)
{
	TS_START("setNewPair")
	texLeft1 = texLeft2;
    texRight1 = texRight2;
    texLeft2 = newObjL->image;
    texRight2 = newObjR->image;

	leftTitleText = ofToUpper(newObjL->title);
	rightTitleText = ofToUpper(newObjR->title);
	TS_STOP("setNewPair");
}

void ImageChanger::setState(State s)
{
	state = s;

	switch (state)
	{
	case ImageChanger::ATTRACT:
		if(isEnterSeqDone)
			startRotateSeq();
		else
			startEnterSeq();
		break;
	case ImageChanger::SHUFFLE:
		isEnterSeqDone = false;
		isSeparateAnimDone = false;
		startEnterSeq();
		break;
	case ImageChanger::TRANSITION:
		if(isSeparateAnimDone)
			startSlideDownAnim();
		else
			startSeparateAnim();
		break;
	case ImageChanger::SPLIT:
		isSeparateAnimDone = false;
		startSeparateAnim();
		break;
	case ImageChanger::REVERSE_TRANSITION:
		isSlideUpDone = false;
		startSlideUpAnim();
		break;
	case ImageChanger::AWAKE:
		ofLogNotice("ImageChanger") << "Awake";
		startTimeout();
		timeout.animateFromTo(1.0, 1.0);
		break;
	default:
		break;
	}
}

void ImageChanger::update()
{
	switch (state)
	{
	case ImageChanger::ATTRACT:
		if(isEnterSeqDone)
			rotateAnim.update(DT);
		else
			updateEnterSeq();
		break;
	case ImageChanger::SHUFFLE:
		if(isEnterSeqDone)
			timeout.update(DT);
		else
			updateEnterSeq();
		break;
	case ImageChanger::TRANSITION:
		if(isSeparateAnimDone)
			slideDownAnim.update(DT);
		else
			separateAnim.update(DT);
		break;
	case ImageChanger::SPLIT:{
		if(isSeparateAnimDone){
			if(isTouched){
				togetherAnim.update(DT);
			}else{
				if(isTimeoutDone)
					togetherAnim.update(DT);
				else
					timeout.update(DT);}
		}else {
			separateAnim.update(DT);
		}
		break;}
	case ImageChanger::REVERSE_TRANSITION:
		if(isSlideUpDone)
			togetherAnim.update(DT);
		else
			slideUpAnim.update(DT);
		break;
	case ImageChanger::AWAKE:
		timeout.update(DT);
		break;
	default:
		break;
	}
}

void ImageChanger::draw()
{
	switch (state)
	{
	case ImageChanger::ATTRACT:
		if(isEnterSeqDone){
			 drawRotateAnim(texLeft2, texRight2);
		}else {
			drawExitAnim(texLeft1, texRight1);
			drawEnterAnim(texLeft2, texRight2);
		}
		break;
	case ImageChanger::SHUFFLE:
		if(isEnterSeqDone){
			drawShuffleTimeout(texLeft2, texRight2);
		}else {
			drawExitAnim(texLeft1, texRight1);
			drawEnterAnim(texLeft2, texRight2);
		}
		break;
	case ImageChanger::TRANSITION:
		if(isSeparateAnimDone)
			drawSlideDownAnim(texLeft2, texRight2);
		else
			drawSeparateAnim(texLeft2, texRight2);
		break;
	case ImageChanger::SPLIT:{
		if(isSeparateAnimDone){
			if(isTouched){
				drawTogetherAnim(texLeft2, texRight2);
			}else{
				if(isTimeoutDone)
					drawTogetherAnim(texLeft2, texRight2);
				else
					drawSeparateTimeout(texLeft2, texRight2);
			}
		}else {
			drawSeparateAnim(texLeft2, texRight2);}
		break;}
	case ImageChanger::REVERSE_TRANSITION:
		if(isSlideUpDone)
			drawTogetherAnim(texLeft2, texRight2);
		else
			drawSlideUpAnim(texLeft2, texRight2);
		break;
	case ImageChanger::AWAKE:
		drawShuffleTimeout(texLeft2, texRight2);
		break;
	default:
		break;
	}
}

void ImageChanger::onEnterSeqDone(ofxAnimatable::AnimationEvent& animEvent)
{
	isEnterSeqDone = true;

	switch (state)
	{
	case ImageChanger::ATTRACT:
		startRotateSeq();
		break;
	case ImageChanger::SHUFFLE:
		startTimeout();
		timeout.animateFromTo(1.0, 1.0);
		break;
	default:
		break;
	}
}

void ImageChanger::onRotateSeqDone()
{
	isEnterSeqDone = false;

	switch (state)
	{
	case ImageChanger::ATTRACT:
		startEnterSeq();
		break;
	case ImageChanger::SHUFFLE:
		startEnterSeq();
		break;
	default:
		break;
	}
}

void ImageChanger::onSeparateAnimDone(ofxAnimatable::AnimationEvent& animEvent)
{
	isSeparateAnimDone = true;
	isTouched = false;

	switch (state)
	{
	case ImageChanger::TRANSITION:
		startSlideDownAnim();
		break;
	case ImageChanger::SPLIT:
		startTimeout();
		timeout.animateFromTo(0.5, 0.5);
		break;
	default:
		break;
	}
}

void ImageChanger::onTimeoutDone(ofxAnimatable::AnimationEvent& animEvent)
{
	isTimeoutDone = true;

	switch (state)
	{
	case ImageChanger::SHUFFLE:
		isSeparateAnimDone = false;	
		setState(ATTRACT);
		break;
	case ImageChanger::SPLIT:
		isEnterSeqDone = true;
		startTogetherAnim();
		break;
	case ImageChanger::AWAKE:
		isSeparateAnimDone = false;
		isEnterSeqDone = true;
		setState(ATTRACT);
		break;
	default:
		break;
	}
}

void ImageChanger::onSlideUpDone(ofxAnimatable::AnimationEvent& animEvent)
{
	//this only happens in REVERSE_TRANSITION state
	isSlideUpDone = true;
	startTogetherAnim();
}

void ImageChanger::startEnterSeq()
{
	ofLogNotice("ImageChanger") << "Enter/exit";
	isAttractSeqDone = false;
	startExitAnim();
    startEnterAnim();
}

void ImageChanger::updateEnterSeq()
{
	exitAnimUpdate();
    enterAnimUpdate();
}

//----Enter Animation -----------------------------------------------------------------------------------------

void ImageChanger::startEnterAnim()
{
    leftEnterAnim.reset(-threshold);
    leftEnterAnim.setCurve(AppParams::enterAnimCurve);
    leftEnterAnim.setDuration(AppParams::enterExitDuration);
    leftEnterAnim.animateTo(0);
    
    rightEnterAnim.reset(threshold);
    rightEnterAnim.setCurve(AppParams::enterAnimCurve);
    rightEnterAnim.setDuration(AppParams::enterExitDuration);
    rightEnterAnim.animateTo(0);
    
    alphaFadeIn.setAlphaOnly(0);
    alphaFadeIn.setCurve(AppParams::fadeAnimCurve);
    alphaFadeIn.setDuration(AppParams::enterExitDuration);
    alphaFadeIn.fadeIn();

	textAlpha.reset(0.0);
	textAlpha.setCurve(EASE_IN_EASE_OUT);
	textAlpha.setDuration(AppParams::enterExitDuration);
	textAlpha.animateTo(1.0);
}

void ImageChanger::enterAnimUpdate()
{
    leftEnterAnim.update(DT);
    rightEnterAnim.update(DT);
    alphaFadeIn.update(DT);
	textAlpha.update(DT);
}

void ImageChanger::drawEnterAnim(ofTexture *texLeft, ofTexture *texRight)
{
    ofPushStyle();
    ofSetColor(255, 255, 255, alphaFadeIn.getCurrentColor().a);
    //left - moves down
    texLeft->drawSubsection(0, leftEnterAnim.val(), w/2, h, 0, 0);
    
    //right - moves up
    texRight->drawSubsection(w/2, rightEnterAnim.val(), w/2, h, w/2, 0);
    ofPopStyle();

	//plus sign
	drawShadowPlus(shadowAlpha);
}

//----Exit Animation-------------------------------------------------------------------------------------------

void ImageChanger::startExitAnim()
{
    leftExitAnim.reset(0);
    leftExitAnim.setCurve(AppParams::enterAnimCurve);
    leftExitAnim.setDuration(AppParams::enterExitDuration);
    leftExitAnim.animateTo(threshold);
    
    rightExitAnim.reset(0);
    rightExitAnim.setCurve(AppParams::enterAnimCurve);
    rightExitAnim.setDuration(AppParams::enterExitDuration);
    rightExitAnim.animateTo(-threshold);
    
    alphaFadeOut.setAlphaOnly(255);
    alphaFadeOut.setCurve(AppParams::fadeAnimCurve);
    alphaFadeOut.setDuration(AppParams::enterExitDuration);
    alphaFadeOut.fadeOut();
}

void ImageChanger::exitAnimUpdate()
{
    leftExitAnim.update(DT);
    rightExitAnim.update(DT);
    alphaFadeOut.update(DT);
}

void ImageChanger::drawExitAnim(ofTexture *texLeft, ofTexture *texRight)
{
    ofPushStyle();
    ofSetColor(255, 255, 255, alphaFadeOut.getCurrentColor().a);
    //left - moves down
    texLeft->drawSubsection(0, leftExitAnim.val(), w/2, h, 0, 0);
    
    //right - moves up
    texRight->drawSubsection(w/2, rightExitAnim.val(), w/2, h, w/2, 0);
    ofPopStyle();

	//plus sign
	drawShadowPlus(shadowAlpha);
}

//----Rotate Animation-----------------------------------------------------------------------------------------

void ImageChanger::startRotateSeq()
{
	ofLogNotice("ImageChanger") << "Rotate";
	rotateAnim.reset(0.0);
    rotateAnim.setCurve(AppParams::rotateAnimCurve);
    rotateAnim.setDuration(4.0);
	rotateAnim.animateTo(360.0);
}

void ImageChanger::speedUpRotateAnim()	//finish rotation quickly in order to shuffle
{
	isEnterSeqDone = false;
	setState(SHUFFLE);
}

void ImageChanger::drawRotateAnim(ofTexture *texLeft, ofTexture *texRight)
{
    ofPushStyle();
    ofPushMatrix();
    
    ofPushMatrix();
    container.begin();
    ofClear(0.0f, 0.0f, 0.0f, 0.0f);
    ofTranslate(w/2, h/2);
   // ofRotate(rotateAnim.val());
	float myRotation = rotateAnim.val();
	ofRotate(myRotation);
    ofTranslate(-w/2, -h/2);
    
    glScissor(0, 0, w/2, h);
    glEnable(GL_SCISSOR_TEST);
    texLeft->draw(0, 0);
    glDisable(GL_SCISSOR_TEST);
    
    glScissor(w/2, 0, w/2, h);
    glEnable(GL_SCISSOR_TEST);
    texRight->draw(0, 0);
    glDisable(GL_SCISSOR_TEST);
    container.end();
    
    ofTranslate(w/2, h/2);
   // ofRotate(-rotateAnim.val());
	ofRotate(-myRotation);
    ofTranslate(-w/2, -h/2);
    container.draw(0, 0);
    ofPopMatrix();

    ofPopMatrix();
    ofPopStyle();
	
	//text with drop shadow
	drawShadowTextAndPlus(shadowAlpha);
}


//----Separate/Shrink Animation-------------------------------------------------------------------------------------

void ImageChanger::startSeparateAnim()
{
	ofLogNotice("ImageChanger") << "Separate";
	separateAnim.reset(1.0);
	separateAnim.setCurve(EASE_IN_EASE_OUT);
	separateAnim.setDuration(1.0);
	separateAnim.animateFromTo(1.0, 0.5);
}

void ImageChanger::drawSeparateAnim(ofTexture *texLeft, ofTexture *texRight)
{
	//ofRectangle r = ofRectangle(0, -0.5*h*(separateAnim.val()-1), w/2, h);
	texLeft->drawSubsection(0, -0.5*h*(separateAnim.val()-1), w/2, h, 0, 0,
		w/2*(1/separateAnim.val()), h*(1/separateAnim.val()));
	
	texRight->drawSubsection(w/2, -0.5*h*(separateAnim.val()-1), w/2, h, w/2*(2*separateAnim.val()-1), 0,
		w/2*(1/separateAnim.val()), h*(1/separateAnim.val()));
	
	texPlus->draw((w-texPlus->getWidth())/2, (h-texPlus->getHeight())/2, texPlus->getWidth(), 
		texPlus->getHeight());

	/*ofNoFill();
	ofRect(r);
	ofFill();*/
}

void ImageChanger::startTogetherAnim()
{
	ofLogNotice("ImageChanger") << "Together";
	togetherAnim.reset(0.5);
	togetherAnim.setCurve(EASE_IN_EASE_OUT);
	togetherAnim.setDuration(1.0);
	togetherAnim.animateFromTo(0.5, 1.0);
}

void ImageChanger::drawTogetherAnim(ofTexture *texLeft, ofTexture *texRight)
{
	texLeft->drawSubsection(0, -0.5*h*(togetherAnim.val()-1), w/2, h, 0, 0,
		w/2*(1/togetherAnim.val()), h*(1/togetherAnim.val()));
	
	texRight->drawSubsection(w/2, -0.5*h*(togetherAnim.val()-1), w/2, h, w/2*(2*togetherAnim.val()-1), 0,
		w/2*(1/togetherAnim.val()), h*(1/togetherAnim.val()));
	
	texPlus->draw((w-texPlus->getWidth())/2, (h-texPlus->getHeight())/2, texPlus->getWidth(), 
		texPlus->getHeight());
}


void ImageChanger::startSlideDownAnim()
{
	ofLogNotice("ImageChanger") << "Slide down";
	slideDownAnim.reset(1.0);
	slideDownAnim.setCurve(EASE_IN_EASE_OUT);
	slideDownAnim.setDuration(1.0);
	slideDownAnim.animateFromTo(1.0, 0.5);
}

void ImageChanger::drawSlideDownAnim(ofTexture *texLeft, ofTexture *texRight)
{
	// Derivation of math for placement and shrinkage: 
	// f(x) = mx + b
	// Use point-slope formula to calculate slide down, because shift from 1 to 0.5 will be linear.
	// For example, if initial location of x is 0 and final location is 230:
	// f(1) = 0; f(0.5) = 230		=>		f(1) = 0; f(0.5) = 0.115 [dividing both by h = 2000]
	// m = f(1)-f(0.5)/1-0.5 = (0-0.115)/(1-0.5) = -0.23
	// f(x) = mx + b
	// 0 = -0.23(1) + b
	// b = 0.23	=>	 f(x) = -0.23(x-1)	=>	 -0.23*h*(slideDownAnim.val()-1);
	texLeft->draw(-0.23*w*(slideDownAnim.val()-1), h*(2.179-1.929*slideDownAnim.val()), 
		w/2*(slideDownAnim.val()), h/2*(slideDownAnim.val())); 
	
	texRight->draw(w*(0.77-0.27*slideDownAnim.val()), h*(2.179-1.929*slideDownAnim.val()), 
		w/2*(slideDownAnim.val()), h/2*(slideDownAnim.val()));

	texPlus->draw((w-texPlus->getWidth())/2, h*(2.169-1.706*slideDownAnim.val()),
		texPlus->getWidth(), texPlus->getHeight());
}

void ImageChanger::startSlideUpAnim()
{
	ofLogNotice("ImageChanger") << "Slide up";
	slideUpAnim.reset(0.5);
	slideUpAnim.setCurve(EASE_IN_EASE_OUT);
	slideUpAnim.setDuration(1.0);
	slideUpAnim.animateFromTo(0.5, 1.0);
}

void ImageChanger::drawSlideUpAnim(ofTexture *texLeft, ofTexture *texRight)
{
	texLeft->draw(-0.23*w*(slideUpAnim.val()-1), h*(2.179-1.929*slideUpAnim.val()), 
		w/2*(slideUpAnim.val()), h/2*(slideUpAnim.val())); 
	
	texRight->draw(w*(0.77-0.27*slideUpAnim.val()), h*(2.179-1.929*slideUpAnim.val()), 
		w/2*(slideUpAnim.val()), h/2*(slideUpAnim.val()));

	texPlus->draw((w-texPlus->getWidth())/2, h*(2.169-1.706*slideUpAnim.val()),
		texPlus->getWidth(), texPlus->getHeight());
}


void ImageChanger::startTimeout()
{
	timeout.reset(0.0);
	timeout.setCurve(LINEAR);
	timeout.setDuration(AppParams::timeoutDuration);
	//set animateFromTo in switch statement - value varies with state
}

void ImageChanger::drawShuffleTimeout(ofTexture *texLeft, ofTexture *texRight)
{
    texLeft->drawSubsection(0, 0, w/2, h, 0, 0);
    texRight->drawSubsection(w/2, 0, w/2, h, w/2, 0);
	drawShadowTextAndPlus(shadowAlpha);
}

void ImageChanger::drawSeparateTimeout(ofTexture *texLeft, ofTexture *texRight)
{
	texLeft->drawSubsection(0, -0.5*h*(separateAnim.val()-1), w/2, h, 0, 0,
		w/2*(1/separateAnim.val()), h*(1/separateAnim.val()));
	
	texRight->drawSubsection(w/2, -0.5*h*(separateAnim.val()-1), w/2, h, w/2*(2*separateAnim.val()-1), 0,
		w/2*(1/separateAnim.val()), h*(1/separateAnim.val()));
	
	texPlus->draw((w-texPlus->getWidth())/2, (h-texPlus->getHeight())/2, texPlus->getWidth(), 
		texPlus->getHeight());
}

void ImageChanger::drawShadowTextAndPlus(float alpha)
{	
	fboBlur.beginDrawScene();
	ofClear(0,0,0,0);
	texPlus->draw((w-texPlus->getWidth())/2, (fboBlur.getSceneFbo().getHeight()-texPlus->getHeight())/2, texPlus->getWidth(), 
		texPlus->getHeight());
	
	// using FontStash, text position lines up with bottom left corner instead of top
	float textYPos = (fboBlur.getSceneFbo().getHeight()+texPlus->getHeight())/2 - 20;

	// centering titles within their halves of the image changer box
	ofPushStyle();
	ofSetColor(225, 225, 225, 255);
	displayFont->setSize(textSize);
	//text wrap for long object names
	if(leftTitleText.length() > 12){
		vector<string> leftTitleVec = ofSplitString(leftTitleText, " ");
		for(int i = 0; i < leftTitleVec.size(); i++){
			float wordWidth = displayFont->stringWidth(leftTitleVec[i]);
			displayFont->draw(leftTitleVec[i], textSize, (0.5*w - wordWidth)/2, textYPos + i*textSize);
		}
	} else {
		float leftTitleWidth = displayFont->stringWidth(leftTitleText);
		displayFont->draw(leftTitleText, textSize, (0.5*w - leftTitleWidth)/2, textYPos);
	}
	//text wrap for long object names
	if(rightTitleText.length() > 12){
		vector<string> rightTitleVec = ofSplitString(rightTitleText, " ");
		for(int i = 0; i < rightTitleVec.size(); i++){
			float wordWidth = displayFont->stringWidth(rightTitleVec[i]);
			displayFont->draw(rightTitleVec[i], textSize, (1.5*w - wordWidth)/2, textYPos + i*textSize);
		}
	} else {
		float rightTitleWidth = displayFont->stringWidth(rightTitleText);
		displayFont->draw(rightTitleText, textSize, (1.5*w - rightTitleWidth)/2, textYPos);
	}
	ofPopStyle();
	fboBlur.endDrawScene();

	fboBlur.performBlur();
	ofSetColor(alpha);
	fboBlur.getBlurredSceneFbo().draw(5, 5 + (h - fboBlur.getSceneFbo().getHeight())/2);
	ofSetColor(255);
	fboBlur.getSceneFbo().draw(0, (h - fboBlur.getSceneFbo().getHeight())/2);
}

void ImageChanger::drawShadowPlus(float alpha)
{
	fboBlur.beginDrawScene();
	ofClear(0,0,0,0);
	texPlus->draw((w-texPlus->getWidth())/2, (fboBlur.getSceneFbo().getHeight()-texPlus->getHeight())/2, texPlus->getWidth(), 
		texPlus->getHeight());
	fboBlur.endDrawScene();

	fboBlur.performBlur();
	ofSetColor(alpha);
	fboBlur.getBlurredSceneFbo().draw(5, 5 + (h - fboBlur.getSceneFbo().getHeight())/2);
	ofSetColor(255);
	fboBlur.getSceneFbo().draw(0, (h - fboBlur.getSceneFbo().getHeight())/2);
}

