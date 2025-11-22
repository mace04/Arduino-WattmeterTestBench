// JavaScript to hide the banner after 15 seconds
document.addEventListener("DOMContentLoaded", () => {
    const banner = document.getElementById("message-banner");
    if (banner) {
        setTimeout(() => {
            banner.style.display = "none";
        }, 10000); // Hide after 15 seconds
    }
});

// Function to add log entries
function addLog(message, type = "info") {
    const logDiv = document.getElementById("log");
    const entry = document.createElement("div");
    entry.className = `log-entry ${type}`;
    const timestamp = new Date().toLocaleTimeString();
    entry.textContent = `[${timestamp}] (${type}) ${message}`;
    logDiv.appendChild(entry);
    // Auto-scroll to bottom
    logDiv.scrollTop = logDiv.scrollHeight;
}

// Connect to Server-Sent Events for real-time logs
document.addEventListener("DOMContentLoaded", () => {
    const eventSource = new EventSource("/events");
    
    eventSource.addEventListener("debug", (event) => {
        addLog(event.data, "info");
    });
    
    eventSource.addEventListener("log", (event) => {
        addLog(event.data, "info");
    });
    
    eventSource.addEventListener("error", (event) => {
        addLog(event.data, "error");
    });
    
    eventSource.addEventListener("warning", (event) => {
        addLog(event.data, "warning");
    });
});
