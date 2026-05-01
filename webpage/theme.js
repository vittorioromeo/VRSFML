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
        apply(next);
        try { localStorage.setItem(STORAGE_KEY, next); } catch (e) {}
    });
})();
