# -*- coding: utf-8 -*-
"""
AWOS Desktop GUI Launcher with Download Support
File: awos_gui.py
"""

import webview
import threading
import time
import sys
import os

def start_flask_server():
    """Start Flask server in background thread"""
    try:
        from awos_server import run_server
        run_server()
    except Exception as e:
        print(f"[ERROR] Server error: {e}")
        sys.exit(1)

class Api:
    """API class for JavaScript to Python communication"""
    
    def save_excel_file(self, filename, base64_data):
        """Save Excel file from base64 data"""
        try:
            import base64
            from pathlib import Path
            
            # Decode base64
            file_data = base64.b64decode(base64_data)
            
            # Get Downloads folder
            downloads_folder = Path.home() / "Downloads"
            filepath = downloads_folder / filename
            
            # Save file
            with open(filepath, 'wb') as f:
                f.write(file_data)
            
            print(f"[SUCCESS] Saved file to: {filepath}")
            return {'success': True, 'filepath': str(filepath)}
            
        except Exception as e:
            print(f"[ERROR] Save failed: {e}")
            return {'success': False, 'error': str(e)}

def start_gui():
    """Start the desktop GUI application"""
    
    # Start Flask server in background
    server_thread = threading.Thread(target=start_flask_server, daemon=True)
    server_thread.start()
    
    # Wait for server to initialize
    print("[WAIT] Waiting for server to start...")
    time.sleep(3)
    
    # Check if server is running
    import requests
    max_retries = 10
    for i in range(max_retries):
        try:
            response = requests.get('http://localhost:3000/api/health', timeout=2)
            if response.status_code == 200:
                print("[OK] Server is ready!")
                break
        except:
            if i < max_retries - 1:
                print(f"[WAIT] Server starting... ({i+1}/{max_retries})")
                time.sleep(1)
            else:
                print("[ERROR] Could not connect to server!")
                sys.exit(1)
    
    # Create API instance
    api = Api()
    
    # Create desktop window with enhanced settings
    print("[GUI] Opening desktop window...")
    
    window = webview.create_window(
        title='AWOS Weather Station',
        url='http://localhost:3000',
        width=1400,
        height=900,
        resizable=True,
        fullscreen=False,
        min_size=(1024, 768),
        background_color='#667BC6',
        # Enable downloads
        confirm_close=False,
        # Expose API to JavaScript
        js_api=api
    )
    
    # Add download handler
    def on_loaded():
        """Execute after page loads"""
        # Inject download helper into page
        window.evaluate_js("""
            // Add Python API helper for downloads
            window.saveExcelToDisk = function(filename, base64Data) {
                if (window.pywebview && window.pywebview.api) {
                    return window.pywebview.api.save_excel_file(filename, base64Data);
                }
                return {success: false, error: 'PyWebView API not available'};
            };
            console.log('PyWebView download API injected');
        """)
    
    window.events.loaded += on_loaded
    
    webview.start(debug=True)  # Enable debug mode to see errors
    
    print("\n[EXIT] Application closed")

if __name__ == '__main__':
    print("=" * 60)
    print("       AWOS Weather Station Desktop App")
    print("=" * 60)
    print()
    
    start_gui()