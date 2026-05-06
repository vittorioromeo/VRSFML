// Minimal theme toggle: persists choice in localStorage, falls back to OS preference.
(function () {
    var STORAGE_KEY = "vrsfml-theme";
    var root = document.documentElement;
    var btn = document.getElementById("theme-toggle");
    if (!btn) return;

    function getPreferred() {
        var stored = null;
        try { stored = localStorage.getItem(STORAGE_KEY); } catch (e) {}
        if (stored === "light" || stored === "dark") return stored;
        return window.matchMedia &&
               window.matchMedia("(prefers-color-scheme: light)").matches
            ? "light" : "dark";
    }

    function apply(theme) {
        root.setAttribute("data-theme", theme);
        btn.setAttribute("aria-pressed", theme === "dark" ? "true" : "false");

        // Switch the active highlight.js theme stylesheet.
        var hjDark  = document.getElementById("hljs-dark");
        var hjLight = document.getElementById("hljs-light");
        if (hjDark)  hjDark.disabled  = (theme === "light");
        if (hjLight) hjLight.disabled = (theme === "dark");
    }

    apply(getPreferred());

    btn.addEventListener("click", function () {
        var next = root.getAttribute("data-theme") === "dark" ? "light" : "dark";

        var commit = function () {
            apply(next);
            try { localStorage.setItem(STORAGE_KEY, next); } catch (e) {}
        };

        // Same-Document View Transitions API: the browser snapshots the
        // current state, runs the mutation, snapshots the new state, then
        // crossfades between the two. Far more reliable than per-property
        // CSS transitions for theme flips, because every visible change
        // (including ones the CSS `transition` rule doesn't list) animates
        // uniformly. Same approach as vittorioromeo.com.
        // Falls back to instant-apply on Safari < 18 / older Firefox.
        if (typeof document.startViewTransition === "function") {
            document.startViewTransition(commit);
        } else {
            commit();
        }
    });
})();
