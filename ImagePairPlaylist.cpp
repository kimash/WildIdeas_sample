//
//  ImagePairPlaylist.cpp
//  WildIdeasAttractLoop
//
//  Created by Kim Ash on 4/17/15.
//
//

#include "ImagePairPlaylist.h"
#include "WIObject.h"
#include "WISketchObject.h"
#include <algorithm>
#include <string>

ImagePairPlaylist::ImagePairPlaylist() 
{
	ofAddListener(SketchManager::one().doneParsing, this, &ImagePairPlaylist::onSketchParseDone);
}

void ImagePairPlaylist::loadPlaylist()
{
	//load all possible object pairs into playlist
	for(int i = 0; i < ContentManager::one().getNumLeftObjects(); i++){
		for(int j = 0; j < ContentManager::one().getNumRightObjects(); j++){
			addToPlaylist(ContentManager::one().getLeftObjectAt(i), 
				ContentManager::one().getRightObjectAt(j));
		}
	}

	//shuffle the playlist
	std::random_shuffle(playlist.begin(), playlist.end());

	ofLogNotice("ImagePairPlaylist") << "Loaded and shuffled playlist: " << playlist.size() 
		<< " total pairs";
}

void ImagePairPlaylist::addToPlaylist(WIObject* leftObj_, WIObject* rightObj_)
{
    ImagePair imgPair;
    
    imgPair.leftObject = leftObj_;
    imgPair.rightObject = rightObj_;
	imgPair.isFeatured = false;
    
    playlist.push_back(imgPair);
}

void ImagePairPlaylist::onSketchParseDone(SketchManager::parsingDoneArgs& args)
{
	ofLogNotice("ImagePairPlaylist") << "Sketch parse done!";
	addSvgStringsToPairs();
}

void ImagePairPlaylist::addSvgStringsToPairs()
{
	for(int i = 0; i < playlist.size(); i++) {
		for(int j = 0; j < SketchManager::one().getNumSketches(); j++) {
			WISketchObject* sketch = SketchManager::one().getSketchAt(j);

			//need to accomodate possible switching of left/right categories
			if((sketch->leftObjectId.compare(playlist[i].leftObject->id) == 0 &&
				sketch->rightObjectId.compare(playlist[i].rightObject->id) == 0) ||
				(sketch->leftObjectId.compare(playlist[i].rightObject->id) == 0 &&
				sketch->rightObjectId.compare(playlist[i].leftObject->id) == 0)) {
					playlist[i].svgStrings.push_back(sketch->svg);
					//if any sketch is featured, then ImagePair will be featured in attract loop
					//once we set ImagePair as featured, it cannot be un-featured until restart
					if(!playlist[i].isFeatured){
						playlist[i].isFeatured = sketch->isFeatured;
					}
			}
		}
	}

	for(int i = 0; i < playlist.size(); i++) {
		ImagePair pair = *getPair(i);
		if(pair.svgStrings.size() > 0){
			if(pair.isFeatured){
				featuredPlaylist.push_back(pair);
				ofLogNotice("ImagePairPlaylist") << "Pair " << pair.leftObject->title
				<< "/" << pair.rightObject->title << " is featured";
			}
			ofLogNotice("ImagePairPlaylist") << "Pair " << pair.leftObject->title
				<< "/" << pair.rightObject->title << " has " << pair.svgStrings.size()
				<< " SVGs";
		}
	}

	//shuffle the featured playlist
	std::random_shuffle(featuredPlaylist.begin(), featuredPlaylist.end());

	ofLogNotice("ImagePairPlaylist") << "Loaded and shuffled featured playlist: " <<
		featuredPlaylist.size() << " featured pairs";

	PlaylistsLoadedArgs plArgs;
	plArgs.playlistSize = playlist.size();
	plArgs.featuredPlaylistSize = featuredPlaylist.size();

	ofNotifyEvent(playlistsLoaded, plArgs, this);
}

WIObject* ImagePairPlaylist::getLeftObjAt(int index)
{
    ImagePair* myPair = getPair(index);
    WIObject* leftObj = myPair->leftObject;
    return leftObj;
}

WIObject* ImagePairPlaylist::getRightObjAt(int index)
{
    ImagePair* myPair = getPair(index);
    WIObject* rightObj = myPair->rightObject;
    return rightObj;
}

WIObject* ImagePairPlaylist::getFeaturedLeftObjAt(int index)
{
	ImagePair* myPair = getFeaturedPair(index);
	return myPair->leftObject;
}

WIObject* ImagePairPlaylist::getFeaturedRightObjAt(int index)
{
	ImagePair* myPair = getFeaturedPair(index);
	return myPair->rightObject;
}

int ImagePairPlaylist::getSvgListSize(int index)
{
	ImagePair* myPair = getPair(index);
	return myPair->svgStrings.size();
}

ImagePairPlaylist::ImagePair* ImagePairPlaylist::getRandomPair()
{
	int index = floor(ofRandom(0, playlist.size()));
	return &playlist[index];
}

ImagePairPlaylist::ImagePair* ImagePairPlaylist::getFeaturedPair(int index)
{
	if(featuredPlaylist.empty()){
		if(!playlist.empty()){
			return &playlist[index];
		} else {
			return NULL;
		}
	} else {
		return &featuredPlaylist[index];
	}
}

int ImagePairPlaylist::getFeaturedPlaylistSize()
{
	if(featuredPlaylist.empty()){
		return playlist.size();
	} else {
		return featuredPlaylist.size();
	}
}
