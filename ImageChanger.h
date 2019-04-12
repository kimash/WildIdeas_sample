//
//  ImageChanger.h
//  WildIdeasAttractLoop
//
//  Created by Kim Ash on 4/16/15.
//
//

#ifndef __WildIdeasAttractLoop__ImageChanger__
#define __WildIdeasAttractLoop__ImageChanger__

#include <iostream>
#include "ofMain.h"
#include "ofxAnimatableOfColor.h"
#include "GComponent.h"
#include "ofxFontStash.h"
#include "WIObject.h"
#include "ofxFboBlur.h"

class ImageChanger: public GComponent {

public:
	~ImageChanger();
    void setup(WIObject* objLeft1, WIObject* objRight1, WIObject* objLeft2, WIObject* objRight2, float x, float y);
    void setNewPair(WIObject* newObjL, WIObject* newObjR);    //changes out one of the pairs

	enum State{
		ATTRACT,
		SHUFFLE,
		TRANSITION,
		SPLIT,
		REVERSE_TRANSITION,
		AWAKE
	};

	void setState(State s);
	State getState() {return state;}
	void update();
	void draw();

	//animation callbacks
	void onEnterSeqDone(ofxAnimatable::AnimationEvent& animEvent);
	void onRotateSeqDone();
	void onSeparateAnimDone(ofxAnimatable::AnimationEvent& animEvent);
	void onTimeoutDone(ofxAnimatable::AnimationEvent& animEvent);
	void onSlideUpDone(ofxAnimatable::AnimationEvent& animEvent);
	
	//enter sequence - includes enterAnim, exitAnim, alphaFadeIn, alphaFadeOut
	void startEnterSeq();
	void updateEnterSeq();

	//rotate sequence
	void startRotateSeq();
    
    //individual animations
    void startEnterAnim();
    void enterAnimUpdate();
    void drawEnterAnim(ofTexture *texLeft, ofTexture *texRight);

	void startExitAnim();
    void exitAnimUpdate();
    void drawExitAnim(ofTexture *texLeft, ofTexture *texRight);
    
	void speedUpRotateAnim();
    void drawRotateAnim(ofTexture *texLeft, ofTexture *texRight);

	void startSeparateAnim();
	void drawSeparateAnim(ofTexture *texLeft, ofTexture *texRight);

	void startTogetherAnim();
	void drawTogetherAnim(ofTexture *texLeft, ofTexture *texRight);

	void startSlideDownAnim();
	void drawSlideDownAnim(ofTexture *texLeft, ofTexture *texRight);

	void startSlideUpAnim();
	void drawSlideUpAnim(ofTexture *texLeft, ofTexture *texRight);

	void startTimeout();
	void drawShuffleTimeout(ofTexture *texLeft, ofTexture *texRight);
	void drawSeparateTimeout(ofTexture *texLeft, ofTexture *texRight);

	void drawShadowTextAndPlus(float alpha);
	void drawShadowPlus(float alpha);
	
	bool isEnterSeqDone;
	bool isAttractSeqDone;
	bool isSeparateAnimDone;
	bool isTouched;
	bool isTimeoutDone;
	bool isSlideUpDone;
	
protected:
	float x, y, w, h;
    float threshold;
	float textSize;
	float shadowAlpha;
	State state;
    ofTexture *texLeft1, *texRight1, *texLeft2, *texRight2;
	ofxFontStash *displayFont;
	string leftTitleText, rightTitleText;
	ofTexture* texPlus;
    
    ofFbo container;    //switch to using drawSubsection eventually?
	ofxFboBlur fboBlur;
    
public:
    //enter
    ofxAnimatableFloat leftEnterAnim;
    ofxAnimatableFloat rightEnterAnim;
    ofxAnimatableOfColor alphaFadeIn;
	ofxAnimatableFloat textAlpha;
    
    //rotate
    ofxAnimatableFloat rotateAnim;
    
    //exit
    ofxAnimatableFloat leftExitAnim;
    ofxAnimatableFloat rightExitAnim;
    ofxAnimatableOfColor alphaFadeOut;

	//separate and together
	ofxAnimatableFloat separateAnim;
	ofxAnimatableFloat togetherAnim;

	//slide down and slide up
	ofxAnimatableFloat slideDownAnim;
	ofxAnimatableFloat slideUpAnim;

	//timeout
	ofxAnimatableFloat timeout;
};



#endif /* defined(__WildIdeasAttractLoop__ImageChanger__) */
