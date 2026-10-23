// Shared helpers for the webserv demo pages.

var NAV_LINKS = [
    { href: "/index.html",  label: "Home" },
    { href: "/static.html", label: "Static" },
    { href: "/get.html",    label: "GET" },
    { href: "/post.html",   label: "POST" },
    { href: "/delete.html", label: "DELETE" },
    { href: "/cgi.html",    label: "CGI" },
    { href: "/status.html", label: "Status codes" }
];

var REASONS = {
    200: "OK", 201: "Created", 204: "No Content",
    301: "Moved Permanently", 302: "Found", 304: "Not Modified",
    400: "Bad Request", 403: "Forbidden", 404: "Not Found",
    405: "Method Not Allowed", 408: "Request Timeout", 411: "Length Required",
    413: "Payload Too Large", 414: "URI Too Long", 415: "Unsupported Media Type",
    500: "Internal Server Error", 501: "Not Implemented", 502: "Bad Gateway",
    503: "Service Unavailable", 504: "Gateway Timeout", 505: "HTTP Version Not Supported"
};

// Build the header and footer so every page shares the same nav.
function buildLayout() {
    var path = window.location.pathname;
    if (path === "/") path = "/index.html";

    var header = document.getElementById("site-header");
    if (header) {
        var items = NAV_LINKS.map(function (l) {
            var cls = l.href === path ? ' class="active"' : "";
            return '<li><a href="' + l.href + '"' + cls + ">" + l.label + "</a></li>";
        }).join("");
        header.className = "site-header";
        header.innerHTML =
            '<div class="inner">' +
            '<a class="logo" href="/"><span class="dot"></span>webserv</a>' +
            "<nav><ul>" + items + "</ul></nav></div>";
    }

    var footer = document.getElementById("site-footer");
    if (footer) {
        footer.innerHTML = "webserv &middot; a 42 project &middot; served by our own C++ HTTP server";
    }
}

function escapeHtml(text) {
    return String(text)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

function statusClass(code) {
    if (!code) return "none";
    return "s" + String(code).charAt(0);
}

function statusBadge(code, text) {
    if (!code) return '<span class="badge none">no response</span>';
    var reason = text || REASONS[code] || "";
    return '<span class="badge ' + statusClass(code) + '">' + code + " " + escapeHtml(reason) + "</span>";
}

function methodBadge(method) {
    return '<span class="badge ' + method.toLowerCase() + '">' + method + "</span>";
}

// Send a request and return a summary object. Never throws.
// Result: { ok, status, statusText, headers, body, ms, error }
function sendRequest(method, url, body, headers) {
    var start = performance.now();
    var opts = { method: method, cache: "no-store" };
    if (body !== undefined && body !== null) opts.body = body;
    if (headers) opts.headers = headers;

    return fetch(url, opts).then(function (res) {
        var headerText = "";
        res.headers.forEach(function (value, key) {
            headerText += key + ": " + value + "\n";
        });
        return res.text().then(function (text) {
            return {
                ok: res.ok,
                status: res.status,
                statusText: res.statusText,
                headers: headerText,
                body: text,
                ms: Math.round(performance.now() - start)
            };
        });
    }).catch(function (err) {
        return {
            ok: false,
            status: 0,
            statusText: "",
            headers: "",
            body: "",
            ms: Math.round(performance.now() - start),
            error: "The server closed the connection without sending a response. " +
                   "This usually means this feature is not handled by the server yet. (" + err.message + ")"
        };
    });
}

// Render a sendRequest() result inside a .result element.
function showResult(el, method, url, r) {
    var html = '<div class="head">' + methodBadge(method) +
        ' <code>' + escapeHtml(url) + "</code> " + statusBadge(r.status, r.statusText) +
        '<span class="time">' + r.ms + " ms</span></div>";
    if (r.error) {
        html += "<pre>" + escapeHtml(r.error) + "</pre>";
    } else {
        if (r.headers) html += '<pre class="headers">' + escapeHtml(r.headers) + "</pre>";
        var preview = r.body.length > 4000 ? r.body.slice(0, 4000) + "\n... (truncated)" : r.body;
        html += "<pre>" + (preview ? escapeHtml(preview) : '<span class="muted">(empty body)</span>') + "</pre>";
    }
    el.innerHTML = html;
    el.classList.add("show");
}

// Shortcut: send a request and render it into the element with this id.
function runInto(resultId, method, url, body, headers) {
    var el = document.getElementById(resultId);
    el.innerHTML = '<div class="head">' + methodBadge(method) + " <code>" + escapeHtml(url) +
        '</code> <span class="muted">sending...</span></div>';
    el.classList.add("show");
    return sendRequest(method, url, body, headers).then(function (r) {
        showResult(el, method, url, r);
        return r;
    });
}

document.addEventListener("DOMContentLoaded", buildLayout);
