#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
USING_NS_CC;

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

	HudLayer* hud = new HudLayer();
	hud->init();
	scene->addChild(hud);
	layer->_hud = hud;

    // return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("pickup.caf");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("hit.caf");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("move.caf");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("TileMap.caf");

	this->setTouchEnabled(true);
	_numCollected = 0;
	_tileMap = new CCTMXTiledMap();
	_tileMap->initWithTMXFile("TileMap.tmx");
	_background = _tileMap->layerNamed("Background");
	this->addChild(_tileMap);
	_foreground = _tileMap->layerNamed("Foreground");
	_meta = _tileMap->layerNamed("Meta");
	_meta->setVisible(false);


	
	CCTMXObjectGroup* objectGroup = _tileMap->objectGroupNamed("Objects");
	if (objectGroup == NULL) {
		CCLog("tile map has no objects object layer.");
		return false;
	}
	CCDictionary* spawnPoint = objectGroup->objectNamed("SpawnPoint");
	int x = ((CCString)*spawnPoint->valueForKey("x")).intValue();
	int y = ((CCString)*spawnPoint->valueForKey("y")).intValue();
	_player = new CCSprite();
	_player->initWithFile("Player.png");
	_player->setPosition(ccp(x, y));
	this->addChild(_player);
	this->setViewPointCenter(_player->getPosition());
	
	
	return true;
}


void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    CCDirector::sharedDirector()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void HelloWorld::setViewPointCenter(CCPoint position) {
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	int x = MAX(position.x, winSize.width/2);
	int y = MAX(position.y, winSize.height/2);
	x = MIN(x, (_tileMap->getMapSize().width*this->_tileMap->getTileSize().width) - winSize.width/2);
	y = MIN(y, (_tileMap->getMapSize().height*this->_tileMap->getTileSize().height) - winSize.height/2);
	CCPoint actualPosition = ccp(x, y);
	CCPoint centerOfView = ccp(winSize.width/2, winSize.height/2);
	CCPoint viewPoint = ccpSub(centerOfView, actualPosition);
	this->setPosition(viewPoint);
}

void HelloWorld::registerWithTouchDispatcher() {
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
}

bool HelloWorld::ccTouchBegan(CCTouch* touch, CCEvent* event) {
	return true;
}

void HelloWorld::setPlayerPosition(CCPoint position) {
	CCPoint tileCoord = this->tileCoordForPosition(position);
	int tileGid = _meta->tileGIDAt(tileCoord);
	if (tileGid) {
		CCDictionary* properties = _tileMap->propertiesForGID(tileGid);
		if (properties) {
			CCString* collision = new CCString();
			*collision = *properties->valueForKey("Collidable");
			if (collision && (collision->compare("True") == 0)) {
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("hit.caf");
				return;
			}
			CCString *collectible = new CCString();
			*collectible = *properties->valueForKey("Collectable");
			if (collectible && (collectible->compare("True") == 0)) {
				_meta->removeTileAt(tileCoord);
				_foreground->removeTileAt(tileCoord);
				_numCollected++;
				_hud->numCollectedChanged(_numCollected);
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("pickup.caf");
			}
		}
	}
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("move.caf");
	_player->setPosition(position);
}

void HelloWorld::ccTouchEnded(CCTouch* touch, CCEvent* event) {
	CCPoint touchLocation = touch->getLocationInView();
	touchLocation = CCDirector::sharedDirector()->convertToGL(touchLocation);
	touchLocation = this->convertToNodeSpace(touchLocation);

	CCPoint playerPos = _player->getPosition();
	CCPoint diff = ccpSub(touchLocation, playerPos);

	if (abs(diff.x) > abs(diff.y)) {
		if (diff.x > 0) {
			playerPos.x += _tileMap->getTileSize().width;
		} else {
			playerPos.x -= _tileMap->getTileSize().width;
		}
	} else {
		if (diff.y > 0) {
			playerPos.y += _tileMap->getTileSize().width;
		} else {
			playerPos.y -= _tileMap->getTileSize().height;
		}
	}
	
	if (playerPos.x <= (_tileMap->getMapSize().width*_tileMap->getTileSize().width)
		&& playerPos.y <= (_tileMap->getMapSize().height*_tileMap->getTileSize().height)
		&& playerPos.y >= 0
		&& playerPos.x >= 0)
	{
		this->setPlayerPosition(playerPos);
	}
}

CCPoint HelloWorld::tileCoordForPosition(CCPoint position) {
	int x = position.x / _tileMap->getTileSize().width;
	int y = ((_tileMap->getMapSize().height*_tileMap->getTileSize().height) - position.y)
		/_tileMap->getTileSize().height;
	return ccp(x, y);
}