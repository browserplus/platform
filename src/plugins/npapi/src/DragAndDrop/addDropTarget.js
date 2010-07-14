function(platformArg, browserArg, id, cbObj) {
  if (!cbObj || !cbObj.onEnter || !cbObj.onExit || !cbObj.onDrop) {
    throw 'error adding target, bogus callback object!';
  }

  var elem = document.getElementById(id);
  var platform = platformArg;
  var browser = browserArg;

  if (elem == null) {
    throw 'can\'t find element with id: \'' + id + '\'';
  }

  if (elem.ondragenter || elem.ondragleave || elem.ondrop) {
    throw 'element already has no-nil value for .ondragenter/.ondragleave/.ondrop -- BrowserPlus cannot attach drop target: \'' + id + '\'';
  }

  var isRealFile = function(f) {
    var hasFileName = false, hasFileSize = false, mutableMembers = false;

    for (var m in f) {
      if (m === 'fileName') hasFileName = true;
      else if (m === 'fileSize') hasFileSize = true;
    }
    try {
      var before = f['fileName'];
      f['fileName'] = '__IMutedYou__';
      if (before !== f['fileName']) mutableMembers = true;
      before = f['fileSize'];
      f['fileSize'] = '__IMutedYou__';
      if (before !== f['fileSize']) mutableMembers = true;
    } catch (e) {
    }
    if (typeof(f) !== 'object' ||
        !f.toString || f.toString.constructor !==  Function ||
        f.toString() !== '[object File]' ||
        f.constructor != File ||
        f.protype != undefined ||
        !hasFileName ||
        !hasFileSize ||
        mutableMembers) {
      return false;
    }

    return true;
  };

  var extractDragItems = function(event) {
    var dt = event.dataTransfer;
    var files = dt.files;
    var uris = [ ];
    if (platform == 'Windows' && browser == 'Firefox') {
      for (var i = 0; i < dt.mozItemCount; i++) {
        uris[i] = dt.mozGetDataAt('text/x-moz-url', i);
      }
    } else if (platform == 'OSX' && browser == 'Safari') {
      var uriText = dt.getData('text/uri-list');
      if (uriText) uris = uriText.split('\n');
    }

    if (uris.length != files.length) {
      throw 'uri/files size mismatch, aborting drop';
    }

    var arr = new Array;
    for (var i = 0; i < files.length; i++) {
      if (!isRealFile(files[i])) {
          throw 'bogus user File data found, possible attack detected!';
      }
      arr.push([ files[i], uris[i] ]);
    }
    return arr;
  };

  var insideCnt = 0;

  /** Adapted from YUI - get the dimensions of a dom node */
  function getDimensions(el) {
    if (!el) { return false; }
    // has to be part of document to have pageXY
    if (el.parentNode === null || el.style.display == 'none') {
      return false;
    }
    var pos = [el.offsetLeft, el.offsetTop];
    var parentNode = el.offsetParent;
    if (parentNode != el) {
      while (parentNode) {
        pos[0] += parentNode.offsetLeft;
        pos[1] += parentNode.offsetTop;
        parentNode = parentNode.offsetParent;
      }
    }
    parentNode = el.parentNode;
    
    // account for any scrolled ancestors
    while (parentNode && parentNode.tagName.toUpperCase() != 'HTML') {
      pos[0] -= parentNode.scrollLeft;
      pos[1] -= parentNode.scrollTop;
      parentNode = parentNode.parentNode;
    }

    return {
      y: parseInt(pos[1]),
      x: parseInt(pos[0]),
      y1: (parseInt(pos[1]) + parseInt(el.offsetHeight)),
      x1: (parseInt(pos[0]) + parseInt(el.offsetWidth))
    };
  }

  function eventInsideTarget(target, event) {
    var cs = getDimensions(target);
    return (cs.x <= event.clientX && event.clientX <= cs.x1 && cs.y <= event.clientY && event.clientY <= cs.y1);
  } 

  elem.ondragenter = function(event) {
    event.stopPropagation(); event.preventDefault(); 
    if (eventInsideTarget(event.currentTarget, event)) {
      if (0 == (insideCnt++)) {
        cbObj.onEnter();
      }
    }
  };

  elem.ondragleave =  function(event) {
    event.stopPropagation(); event.preventDefault(); 
    if (insideCnt > 0) {
      insideCnt--;
      if (!insideCnt || !eventInsideTarget(event.currentTarget, event)) {
        insideCnt = 0;
        cbObj.onExit();
      }
    }
  };

  elem.ondragover = function(event) {
    event.stopPropagation(); event.preventDefault(); 
    var hit = eventInsideTarget(event.currentTarget, event);
    if (hit && insideCnt == 0) {
       insideCnt++; 
       cbObj.onEnter();
    } else if (!hit && insideCnt > 0) {
      if (0 == --insideCnt) cbObj.onExit();
    }
  };

  elem.ondrop = function(event) {
    event.stopPropagation(); event.preventDefault(); 
    insideCnt = 0;
    if (eventInsideTarget(event.currentTarget, event)) {
      cbObj.onDrop(extractDragItems(event));
    }
  };

  return null;
}
