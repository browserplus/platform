function(id) {
  var elem = document.getElementById(id);
  elem.ondrop = elem.ondragover = elem.ondragleave = elem.ondragenter = null;
}