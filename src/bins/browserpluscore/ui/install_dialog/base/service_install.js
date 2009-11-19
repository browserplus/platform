var startFunc = function() {

var YD = YAHOO.util.Dom;
var YE = YAHOO.util.Event;

function human_readable_size(size)
{
    var i, units = ['B','KB','MB','GB','TB'];

    for (i = 0; size > 1024; i++) {
        size /= 1024;
    }

    return Math.round(size*10)/10 + units[i];
}

// set functions for the buttons
YE.addListener('deny', 'click', function(e) {
    YE.preventDefault(e);
    BPDialog.complete((YD.get("permanent_checkbox_input").checked ? "AlwaysDeny" : "Deny"));
    return false;
});

YE.addListener('allow', 'click', function(e) {
    YE.preventDefault(e);
    BPDialog.complete((YD.get("permanent_checkbox_input").checked ? "AlwaysAllow" : "Allow"));
    return false;
});

YE.addListener('permanent_checkbox_input', 'click', function() {
    var chk = YD.get("permanent_checkbox_input").checked;
    if (chk) {
        YD.get("allow").innerHTML = "<%= prompt.alwaysAllow %>";
        YD.get("deny").innerHTML  = "<%= prompt.alwaysDeny %>";
    } else {
        YD.get("allow").innerHTML = "<%= prompt.allow %>";
        YD.get("deny").innerHTML  = "<%= prompt.deny %>";
    }
});

var service_detail_handler = function(e)
{
    var details = YE.getTarget(e).id;
    var container = details + "_container";
    
    var style = YD.getStyle(container, "display");

    if (style == "none") {
        YD.replaceClass(details, "service_detail_hide", "service_detail_show");
        YD.setStyle(container, "display", "block");
    } else {
        YD.replaceClass(details, "service_detail_show", "service_detail_hide");
        YD.setStyle(container, "display", "none");
    }
};

function renderService(id, title, details, services, update, type)
{
    var i, size = 0, cnt = 0, str = "", id1 = id + "_container",
        tin  = '<div class="service_name">{name} {version}</div><div class="service_summary">{summary}</div>',
        tout = 
            '<div class="component">' +
            '   <div class="{type} component_info">' + 
            '       <div class="component_name">{title}</div>' + 
            '       <div id="{id}" class="service_detail service_detail_hide">{details}</div>' + 
            '       <div id="{id1}" style="display:none">{inner}</div>' +
            '   </div>' +
            '</div>';

    for (i = 0; i < services.length; i++) {
        if (services[i].update == update) {
            size += services[i].downloadSize;
            cnt++;
            str += YAHOO.lang.substitute(tin, {name:services[i].title, version:services[i].version, summary:services[i].summary});
        }
    }

    if (cnt === 0) {
        return {size:0,html:''};
    }

    title = YAHOO.lang.substitute(title, {num: cnt, service:(cnt == 1 ? "<%= prompt.service %>" : "<%= prompt.services %>")});

    return { 
        size: size, 
        html: YAHOO.lang.substitute(tout, {id: id, id1: id1, title: title, type: type, details: details, inner: str})
    };
}

// now build up the list of components
try {

    var i, str = "", msg, obj, size = 0,
        args = BPDialog.args(),
        services = args.services,
        perms = args.permissions,
        plats = args.platformUpdates;
        permissionTemplate = 
            '<div class="component">' + 
            '    <div class="perm_component component_info">' +
            '        <div class="component_name">{name}</div>' + 
            '    </div>' +
            '</div>';

    YD.get("requestPermission").innerHTML = YAHOO.lang.substitute("<%= prompt.requestPermission %>", {domain_name:args.domain});
  
    // An ugly and horrid hack to render properly on systems
    // where DPI setting is > 96 (default) and IE8 is not installed
    // (YIB-2897631)
    if ( BPDialog.dpiHack() && screen.deviceXDPI > 96 ) {
        YD.get("description").style.fontSize = "75%";
    }
   
    YD.get("requested_domain").innerHTML = args.domain;
    
    for (i = 0; i < perms.length; i++) {
        str += YAHOO.lang.substitute(permissionTemplate, {'name':perms[i]});
    }


    obj = renderService("sd0", "<%= prompt.updateBrowserPlus %>", "<%= prompt.platformDetails %>", plats, true, "bplus_component");
    str += obj.html;
    size += obj.size;

    obj = renderService("sd1", "<%= prompt.downloadService %>", "<%= prompt.serviceDetails %>", services, false, "add_component");
    str += obj.html;
    size += obj.size;

    obj = renderService("sd2", "<%= prompt.updateService %>", "<%= prompt.serviceDetails %>", services, true, "update_component");
    str += obj.html;
    size += obj.size;

    if (size === 0) {
        YD.get('total_size_text').innerHTML = "<%= prompt.noDownloadRequired %>";
    } else {
        YD.get('total_size_text').innerHTML = "<%= prompt.downloadSize %>";
        YD.get('total_size_number').innerHTML = human_readable_size(size);
    }

    YD.get("component_list").innerHTML = str;
    YE.addListener("sd0", "click", service_detail_handler);
    YE.addListener("sd1", "click", service_detail_handler);
    YE.addListener("sd2", "click", service_detail_handler);
    
    BPDialog.show(607, 355);

} catch (ex) {
    msg = "service_install.js, exception:\n";
    for (i in ex) {
        msg = msg + i + ": " + ex[i] + "\n";
    }
    alert(msg);
    BPDialog.log(msg);
}

};


/* Init function.  Sets language strings then starts rest of interface. */
(function() {
    if (window.addEventListener) {
        window.addEventListener("load", startFunc, true);
    } else if (window.attachEvent) {
        window.attachEvent("onload", startFunc);
    }
})();
