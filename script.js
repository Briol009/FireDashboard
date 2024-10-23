// script.js

document.addEventListener('DOMContentLoaded', () => {
  const toggleButton = document.getElementById('toggle-view');
  const unityContainer = document.getElementById('unity-container');
  const modelViewerContainer = document.getElementById('model-viewer-container');
  const unityIframe = document.getElementById('unity-iframe');
  const webglWarning = document.getElementById('webgl-warning');
  const container = document.querySelector('.container');
  const header = document.querySelector('header');

  // Elements for person count and motion presence
  const personCountElement = document.getElementById('person-count');
  const motionPresenceBox = document.getElementById('motion-presence-box');
  const motionStatusElement = document.getElementById('motion-status');

  // Model Viewer Elements
  const modelViewer = document.querySelector('model-viewer');
  const modelLoadingBar = document.getElementById('model-loading-bar');
  const modelProgressBarFull = document.getElementById('model-progress-bar-full');

  // Resizer Elements
  const leftResizer = document.getElementById('left-resizer');
  const rightResizer = document.getElementById('right-resizer');
  const leftSidebar = document.querySelector('.sidebar.left');
  const rightSidebar = document.querySelector('.sidebar.right');
  const viewer = document.querySelector('.viewer');

  let isUnityVisible = false; // Start with Unity hidden
  let isModelLoaded = false; // Track if the model has been loaded

  // Function to adjust the margin-top of the container based on header height
  function adjustContainerMargin() {
    const headerHeight = header.offsetHeight;
    container.style.marginTop = `${headerHeight}px`;
    container.style.height = `calc(100% - ${headerHeight}px)`;
  }

  // Call the function on load
  adjustContainerMargin();

  // Also adjust on window resize in case the header height changes
  window.addEventListener('resize', adjustContainerMargin);

  // Toggle Button Functionality
  toggleButton.addEventListener('click', () => {
    if (isUnityVisible) {
      // Hide Unity iframe and show model-viewer
      unityContainer.style.display = 'none';
      modelViewerContainer.style.display = 'block';
      toggleButton.textContent = 'Show Unity VR';

      if (!isModelLoaded) {
        // Show the model loading overlay
        modelLoadingBar.style.display = 'flex';
      } else {
        // Hide the model loading overlay
        modelLoadingBar.style.display = 'none';
      }
    } else {
      // Show Unity iframe and hide model-viewer
      unityContainer.style.display = 'block';
      modelViewerContainer.style.display = 'none';
      toggleButton.textContent = 'Show 3D Model';

      // No need to show the Unity loading overlay
    }
    isUnityVisible = !isUnityVisible;
  });

  // When the model is loaded
  modelViewer.addEventListener('load', () => {
    modelLoadingBar.style.display = 'none';
    isModelLoaded = true; // Set the flag to true
  });

  // Variables to store current positions
  let isResizingLeft = false;
  let isResizingRight = false;

  // Mouse down events
  leftResizer.addEventListener('mousedown', function (e) {
    isResizingLeft = true;
    document.body.style.cursor = 'ew-resize';
  });

  rightResizer.addEventListener('mousedown', function (e) {
    isResizingRight = true;
    document.body.style.cursor = 'ew-resize';
  });

  // Mouse move event
  document.addEventListener('mousemove', function (e) {
    if (isResizingLeft) {
      let newWidth = e.clientX - leftSidebar.offsetLeft;
      if (newWidth > 100 && newWidth < 400) {
        leftSidebar.style.width = newWidth + 'px';
      }
    } else if (isResizingRight) {
      let totalWidth = container.clientWidth;
      let newWidth = totalWidth - e.clientX;
      if (newWidth > 100 && newWidth < 400) {
        rightSidebar.style.width = newWidth + 'px';
      }
    }
  });

  // Mouse up event
  document.addEventListener('mouseup', function (e) {
    if (isResizingLeft || isResizingRight) {
      isResizingLeft = false;
      isResizingRight = false;
      document.body.style.cursor = 'default';
    }
  });

  // CORS Proxy URL (replace with your actual function URL)
  const CORS_PROXY_URL = 'https://us-central1-giscloud-436023.cloudfunctions.net/corstest2';

  // Function to fetch data from CORS proxy
  async function fetchData() {
    try {
      const response = await fetch(CORS_PROXY_URL);
      if (response.ok) {
        const data = await response.json();
        updateDashboard(data);
      } else {
        console.error('Failed to fetch data:', response.statusText);
      }
    } catch (error) {
      console.error('Error fetching data:', error);
    }
  }

  // Function to update the dashboard with fetched data
  function updateDashboard(data) {
    const { count, presence } = data;

    // Update People in Room
    if (count !== null && count !== undefined) {
      personCountElement.textContent = count;
    }

    // Update Presence Detection
    if (presence !== null && presence !== undefined) {
      updateMotionPresenceBox(presence);
    }
  }

  // Function to update motion presence box based on presence value
  function updateMotionPresenceBox(presence) {
    if (presence) {
      // Motion detected
      motionPresenceBox.classList.remove('no-motion');
      motionPresenceBox.classList.add('motion-detected');
      motionStatusElement.textContent = 'Motion Detected';
    } else {
      // No motion
      motionPresenceBox.classList.remove('motion-detected');
      motionPresenceBox.classList.add('no-motion');
      motionStatusElement.textContent = 'No Motion';
    }
  }

  // Initial data fetch
  fetchData();

  // Set interval to refresh data every 5 seconds
  setInterval(fetchData, 5000);
});
