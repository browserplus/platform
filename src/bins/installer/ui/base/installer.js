/* change the page */
function setPage(name) {
	document.getElementById("splashPage").style.display = "none";
	document.getElementById("progressPage").style.display = "none";
	document.getElementById("donePage").style.display = "none";
	document.getElementById("errorPage").style.display = "none";
	document.getElementById(name+"Page").style.display = "block";
}

/* enter an error state */
function setError(errStr) {
	setPage("error");
	document.getElementById("specificError").innerHTML = errStr;
}

function pollProgress()
{
	setTimeout(function() {
		s = BPInstaller.state();

		if (s.state === "error") {
			setError(s.desc);
		} else if (s.state === "installing") {
			document.getElementById("statusBarProgress").style.width = String(s.progress) +	"%";
			document.getElementById("progressText").innerHTML = (s.desc || "");
			pollProgress();
		} else if (s.state === "complete") {
			
			// user probably won't see this, but in case the screen is slow to update,
			// finish progress
			document.getElementById("statusBarProgress").style.width = "100%";
			document.getElementById("progressText").innerHTML = "<%= installer.doneTitle %>";
			
			// allDone not exiting on OS X?
			BPInstaller.allDone();

			// just in case, go to done page
			setPage("done");
		}
	}, 100);
}

document.installaction.begin.onclick = function() {
	setPage("progress");
	pollProgress();
	BPInstaller.beginInstall();
};

document.installaction.cancel.onclick = function() {
	BPInstaller.cancelInstall();
};

document.goodbye.alldone.onclick = function() {
	BPInstaller.allDone();
};

document.errorForm.errorButton.onclick = function() {
	BPInstaller.cancelInstall();
};

/* if an error occurred during startup we'll message that to the end user
 * now.	 otherwise we'll show the splash page. */
YAHOO.util.Event.addListener(window, 'load', function() {
  s = BPInstaller.state();	
  if (s.state === "error") {
	setError(s.desc);
  } else {
	setPage("splash");
  } 
});
