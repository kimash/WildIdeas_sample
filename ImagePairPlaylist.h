//
//  ImagePairPlaylist.h
//  WildIdeasAttractLoop
//
//  Created by Kim Ash on 4/17/15.
//
//

#ifndef __WildIdeasAttractLoop__ImagePairPlaylist__
#define __WildIdeasAttractLoop__ImagePairPlaylist__

#include <iostream>
#include "ofMain.h"
#include "WildIdeasContentManager.h"
#include "SketchManager.h"
class WIObject;

class ImagePairPlaylist {
    
public:
	static ImagePairPlaylist& one() {
		static ImagePairPlaylist instance;
		return instance;
	}

    struct ImagePair {
        WIObject* leftObject;
        WIObject* rightObject;
		vector<string> svgStrings;
		bool isFeatured;
    };
    
    void addToPlaylist(WIObject* leftObj_, WIObject* rightObj_);
	void loadPlaylist();
	void addSvgStringsToPairs();

	void onSketchParseDone(SketchManager::parsingDoneArgs& args);

	void addSvgToImagePair(ImagePair* pair, string svg) {pair->svgStrings.push_back(svg);}

	struct PlaylistsLoadedArgs {
		int playlistSize;
		int featuredPlaylistSize;
	};

	ofEvent<PlaylistsLoadedArgs> playlistsLoaded;
    
	WIObject* getLeftObject(ImagePair* pair) {return pair->leftObject;}
	WIObject* getRightObject(ImagePair* pair) {return pair->rightObject;}

	ImagePair* getRandomPair();
	ImagePair* getPair(int index) {return &playlist[index];}
	ImagePair* getFeaturedPair(int index);

    WIObject* getLeftObjAt(int index);
    WIObject* getRightObjAt(int index);
	WIObject* getFeaturedLeftObjAt(int index);
	WIObject* getFeaturedRightObjAt(int index);
    
	int getPlaylistSize() {return playlist.size();}
	int getFeaturedPlaylistSize(); 
	int getSvgListSize(int index);
	int getSvgListSize(ImagePair* pair) {return pair->svgStrings.size();}

private:
	ImagePairPlaylist();

	vector<ImagePair> playlist;
	vector<ImagePair> featuredPlaylist;
};

#endif /* defined(__WildIdeasAttractLoop__ImagePairPlaylist__) */
