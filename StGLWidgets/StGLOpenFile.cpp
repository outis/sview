/**
 * Copyright © 2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLOpenFile.h>

#include <StGLWidgets/StGLMenu.h>
#include <StGLWidgets/StGLMenuItem.h>
#include <StGLWidgets/StGLScrollArea.h>
#include <StGLWidgets/StGLTextureButton.h>

StGLOpenFile::StGLOpenFile(StGLWidget*     theParent,
                           const StString& theTitle,
                           const StString& theCloseText)
: StGLMessageBox(theParent, theTitle, "",
                 theParent->getRoot()->scale(512), theParent->getRoot()->scale(400)),
  myHotList(NULL),
  myList(NULL),
  myFileColor(1.0f, 1.0f, 1.0f, 1.0f),
  myHotColor (1.0f, 1.0f, 1.0f, 1.0f) {
    myToAdjustY = false;
    myIconSize  = myRoot->scaleIcon(16, myIconMargins);

    myHotSizeX = myRoot->scale(100);
    myHotList = new StGLMenu(this, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);
    myHotList->setOpacity(1.0f, true);
    myHotList->setItemWidth(myHotSizeX);
    myHotList->setColor(StGLVec4(0.0f, 0.0f, 0.0f, 0.0f));
    myHotList->changeRectPx().top()   = myMarginTop;
    myHotList->changeRectPx().left()  = myMarginLeft;
    myHotList->changeRectPx().right() = myMarginLeft + myHotSizeX;
    myContent->changeRectPx().left()  = myMarginLeft + myHotSizeX;

    myList = new StGLMenu(myContent, 0, 0, StGLMenu::MENU_VERTICAL_COMPACT);
    myList->setOpacity(1.0f, true);
    myList->setColor(StGLVec4(0.0f, 0.0f, 0.0f, 0.0f));
    myList->setItemWidthMin(myContent->getRectPx().width());

    if(!myRoot->isMobile()) {
        addButton(theCloseText);
    }
}

StGLOpenFile::~StGLOpenFile() {
    StGLContext& aCtx = getContext();
    if(!myTextureFolder.isNull()) {
        for(size_t aTexIter = 0; aTexIter < myTextureFolder->size(); ++aTexIter) {
            myTextureFolder->changeValue(aTexIter).release(aCtx);
        }
        myTextureFolder.nullify();
    }
    if(!myTextureFile.isNull()) {
        for(size_t aTexIter = 0; aTexIter < myTextureFile->size(); ++aTexIter) {
            myTextureFile->changeValue(aTexIter).release(aCtx);
        }
        myTextureFile.nullify();
    }
}

void StGLOpenFile::setMimeList(const StMIMEList& theFilter) {
    myFilter     = theFilter;
    myExtensions = theFilter.getExtensionsList();
}

void StGLOpenFile::doHotItemClick(const size_t theItemId) {
    myItemToLoad = myHotPaths[theItemId];
}

void StGLOpenFile::doFileItemClick(const size_t theItemId) {
    const StFileNode* aNode = myFolder->getValue(theItemId);
    myItemToLoad = aNode->getPath();
}

bool StGLOpenFile::tryUnClick(const StPointD_t& theCursorZo,
                              const int&        theMouseBtn,
                              bool&             isItemUnclicked) {
    bool aRes = StGLMessageBox::tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked);
    if(!myItemToLoad.isEmpty()) {
        StString aPath = myItemToLoad;
        myItemToLoad.clear();
        if(StFolder::isFolder(aPath)) {
            openFolder(aPath);
        } else {
            StHandle<StString> aPathHandle = new StString(aPath);
            signals.onFileSelected.emit(aPathHandle);
            myRoot->destroyWithDelay(this);
        }
    }
    return aRes;
}

void StGLOpenFile::addHotItem(const StString& theTarget,
                              const StString& theName) {
    StString aName = theName;
    if(aName.isEmpty()) {
        StString aFoler;
        StFileNode::getFolderAndFile(theTarget, aFoler, aName);
    }
    if(aName.isEmpty()) {
        aName = theTarget;
    }
    if(aName.isEmpty()) {
        return;
    }
    myHotPaths.add(theTarget);

    StGLMenuItem* anItem = new StGLPassiveMenuItem(myHotList);
    setItemIcon(anItem, myHotColor, true);
    anItem->setText(aName);
    anItem->setTextColor(myHotColor);
    anItem->setUserData(myHotPaths.size() - 1);
    anItem->signals.onItemClick = stSlot(this, &StGLOpenFile::doHotItemClick);

    int aSizeX = anItem->getMargins().left + anItem->computeTextWidth() + anItem->getMargins().right;
    myHotSizeX = stMax(myHotSizeX, aSizeX);

    myContent->changeRectPx().left() = myMarginLeft + myHotSizeX;
    myList->setItemWidthMin(myContent->getRectPx().width());
}

void StGLOpenFile::setItemIcon(StGLMenuItem*   theItem,
                               const StGLVec4& theColor,
                               const bool      theisFolder) {
    if(theItem == NULL) {
        return;
    }

    if(myTextureFolder.isNull()) {
        const StString& anIcon0 = myRoot->getIcon(StGLRootWidget::IconImage_Folder);
        const StString& anIcon1 = myRoot->getIcon(StGLRootWidget::IconImage_File);
        if(!anIcon0.isEmpty()
        && !anIcon1.isEmpty()) {
            myTextureFolder = new StGLTextureArray(1);
            myTextureFile   = new StGLTextureArray(1);
            myTextureFolder->changeValue(0).setName(anIcon0);
            myTextureFile  ->changeValue(0).setName(anIcon1);
        } else {
            return;
        }
    }

    const int aMargin      = myRoot->scale(8);
    const int anIconMargin = myRoot->scale(16 + 8);
    StGLIcon* anIcon = new StGLIcon(theItem, aMargin, 0, StGLCorner(ST_VCORNER_CENTER, ST_HCORNER_LEFT), 0);
    anIcon->setColor(theColor);
    if(theisFolder) {
        anIcon->setExternalTextures(myTextureFolder);
    } else {
        anIcon->setExternalTextures(myTextureFile);
    }
    theItem->setIcon(anIcon);
    theItem->changeMargins().left = anIconMargin + aMargin;
}

void StGLOpenFile::openFolder(const StString& theFolder) {
    myItemToLoad.clear();
    myList->destroyChildren();

    myFolder = new StFolder(theFolder);
    myFolder->init(myExtensions, 1, true);

    const size_t aNbItems = myFolder->size();
    for(size_t anItemIter = 0; anItemIter < aNbItems; ++anItemIter) {
        const StFileNode* aNode = myFolder->getValue(anItemIter);
        StString aName = aNode->getSubPath();
        StGLMenuItem* anItem = new StGLPassiveMenuItem(myList);
        setItemIcon(anItem, myFileColor, aNode->isFolder());
        anItem->setText(aName);
        anItem->setTextColor(myFileColor);

        anItem->setUserData(anItemIter);
        anItem->signals.onItemClick = stSlot(this, &StGLOpenFile::doFileItemClick);

    }
    myList->stglInit();
    stglInit();
}