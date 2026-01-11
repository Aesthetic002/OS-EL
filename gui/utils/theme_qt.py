"""
Theme and Styling for OS-EL Deadlock Detection GUI (PyQt6)

Provides a minimal, professional dark theme with clean aesthetics.
"""

from PyQt6.QtGui import QColor, QFont, QPalette
from PyQt6.QtWidgets import QApplication


# ============================================================================
# Color Palette - Clean Professional Dark Theme
# ============================================================================

COLORS = {
    # Primary colors - Refined indigo
    'primary': '#5c6bc0',           # Softer indigo
    'primary_dark': '#3f51b5',
    'primary_light': '#7986cb',
    'primary_hover': '#7c4dff',
    
    # Secondary colors
    'secondary': '#546e7a',         # Blue-gray
    'secondary_dark': '#455a64',
    'secondary_light': '#78909c',
    
    # Accent colors
    'accent': '#7e57c2',            # Deep purple
    'success': '#4caf50',           # Green
    'warning': '#ff9800',           # Orange
    'error': '#e53935',             # Red
    'info': '#29b6f6',              # Light blue
    
    # Background colors (Dark Theme - deeper contrast)
    'bg_primary': '#121820',        # Deep dark
    'bg_secondary': '#1a2332',      # Card background
    'bg_tertiary': '#243042',       # Elevated surfaces
    'bg_card': '#1e2a3a',
    'bg_hover': '#2a3a4d',
    'bg_input': '#151d28',
    
    # Surface colors
    'surface': '#1e2a3a',
    'surface_light': '#2a3a4d',
    'surface_lighter': '#3a4a5d',
    
    # Text colors
    'text_primary': '#eceff1',
    'text_secondary': '#b0bec5',
    'text_muted': '#607d8b',
    'text_white': '#ffffff',
    'text_on_primary': '#ffffff',
    
    # Border colors
    'border': '#2a3a4d',
    'border_light': '#3a4a5d',
    'border_dark': '#1a2332',
    'border_focus': '#5c6bc0',
    
    # Graph node colors
    'node_process': '#42a5f5',      # Blue
    'node_process_glow': '#64b5f6',
    'node_resource': '#7e57c2',     # Purple
    'node_resource_glow': '#9575cd',
    
    # Graph edge colors
    'edge_request': '#ffb74d',      # Amber - dashed
    'edge_assignment': '#66bb6a',   # Green - solid
    
    # State colors
    'deadlock': '#e53935',
    'deadlock_glow': '#ef5350',
    'safe': '#4caf50',
    'safe_glow': '#66bb6a',
    
    # Gradient colors
    'gradient_start': '#5c6bc0',
    'gradient_end': '#7e57c2',
}


# ============================================================================
# Font Configuration
# ============================================================================

FONTS = {
    'family': 'Segoe UI',
    'family_mono': 'Consolas',
    'size_h1': 18,
    'size_h2': 13,
    'size_h3': 11,
    'size_body': 10,
    'size_small': 9,
    'size_tiny': 8,
}


def get_font(size='body', bold=False, mono=False):
    """Get a QFont with the specified parameters"""
    family = FONTS['family_mono'] if mono else FONTS['family']
    
    # Size mapping
    size_map = {
        'h1': FONTS['size_h1'],
        'h2': FONTS['size_h2'],
        'h3': FONTS['size_h3'],
        'body': FONTS['size_body'],
        'small': FONTS['size_small'],
        'tiny': FONTS['size_tiny'],
    }
    
    font_size = size_map.get(size, FONTS['size_body'])
    
    font = QFont(family, font_size)
    if bold:
        font.setBold(True)
    
    return font


# ============================================================================
# Stylesheet - Minimal Professional Dark Theme
# ============================================================================

STYLESHEET = f"""
/* Global Application Style */
QWidget {{
    background-color: {COLORS['bg_primary']};
    color: {COLORS['text_primary']};
    font-family: '{FONTS['family']}';
    font-size: {FONTS['size_body']}pt;
}}

/* Main Window */
QMainWindow {{
    background-color: {COLORS['bg_primary']};
}}

/* Scroll Area */
QScrollArea {{
    border: none;
    background-color: transparent;
}}

/* Group Box / Frame - Clean card style */
QGroupBox {{
    background-color: {COLORS['bg_card']};
    border: 1px solid {COLORS['border']};
    border-radius: 8px;
    margin-top: 12px;
    padding: 16px;
    padding-top: 24px;
    font-weight: 600;
}}

QGroupBox::title {{
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 12px;
    padding: 2px 8px;
    color: {COLORS['text_primary']};
    font-size: {FONTS['size_body']}pt;
    font-weight: 600;
    background-color: {COLORS['bg_card']};
    border-radius: 4px;
}}

/* Frame */
QFrame {{
    border-radius: 8px;
}}

/* Labels */
QLabel {{
    color: {COLORS['text_primary']};
    background: transparent;
    padding: 2px 0;
}}

QLabel[heading="true"] {{
    font-size: {FONTS['size_h2']}pt;
    font-weight: 600;
    color: {COLORS['text_primary']};
}}

/* Push Buttons - Clean minimal style */
QPushButton {{
    background-color: {COLORS['primary']};
    color: {COLORS['text_white']};
    border: none;
    border-radius: 6px;
    padding: 8px 16px;
    font-weight: 500;
    min-height: 28px;
}}

QPushButton:hover {{
    background-color: {COLORS['primary_light']};
}}

QPushButton:pressed {{
    background-color: {COLORS['primary_dark']};
}}

QPushButton:disabled {{
    background-color: {COLORS['bg_tertiary']};
    color: {COLORS['text_muted']};
}}

QPushButton[secondary="true"] {{
    background-color: transparent;
    border: 1px solid {COLORS['border_light']};
    color: {COLORS['text_secondary']};
}}

QPushButton[secondary="true"]:hover {{
    background-color: {COLORS['bg_hover']};
    border-color: {COLORS['primary']};
    color: {COLORS['text_primary']};
}}

QPushButton[danger="true"] {{
    background-color: {COLORS['error']};
}}

QPushButton[danger="true"]:hover {{
    background-color: #c62828;
}}

QPushButton[success="true"] {{
    background-color: {COLORS['success']};
}}

QPushButton[success="true"]:hover {{
    background-color: #388e3c;
}}

/* Line Edit - Clean input style */
QLineEdit {{
    background-color: {COLORS['bg_input']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 6px;
    padding: 8px 12px;
    selection-background-color: {COLORS['primary']};
}}

QLineEdit:focus {{
    border-color: {COLORS['border_focus']};
    background-color: {COLORS['bg_secondary']};
}}

QLineEdit:disabled {{
    background-color: {COLORS['bg_tertiary']};
    color: {COLORS['text_muted']};
}}

QLineEdit::placeholder {{
    color: {COLORS['text_muted']};
}}

/* Spin Box */
QSpinBox {{
    background-color: {COLORS['bg_input']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 6px;
    padding: 6px 10px;
    min-width: 70px;
}}

QSpinBox:focus {{
    border-color: {COLORS['border_focus']};
}}

QSpinBox::up-button, QSpinBox::down-button {{
    background-color: {COLORS['bg_tertiary']};
    border: none;
    width: 20px;
    border-radius: 3px;
    margin: 2px;
}}

QSpinBox::up-button:hover, QSpinBox::down-button:hover {{
    background-color: {COLORS['primary']};
}}

/* Combo Box */
QComboBox {{
    background-color: {COLORS['bg_input']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 6px;
    padding: 8px 12px;
    min-height: 22px;
}}

QComboBox:focus {{
    border-color: {COLORS['border_focus']};
}}

QComboBox::drop-down {{
    border: none;
    width: 30px;
}}

QComboBox::down-arrow {{
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 6px solid {COLORS['text_secondary']};
    margin-right: 10px;
}}

QComboBox QAbstractItemView {{
    background-color: {COLORS['bg_secondary']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 6px;
    selection-background-color: {COLORS['primary']};
    padding: 4px;
}}

/* Text Edit / Plain Text Edit */
QTextEdit, QPlainTextEdit {{
    background-color: {COLORS['bg_input']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 6px;
    padding: 10px;
    font-family: '{FONTS['family_mono']}';
    font-size: {FONTS['size_small']}pt;
}}

QTextEdit:focus, QPlainTextEdit:focus {{
    border-color: {COLORS['border_focus']};
}}

/* Main Tab Widget (Control/Deadlock) */
QTabWidget::pane {{
    border: none;
    background-color: {COLORS['bg_secondary']};
    border-radius: 0 8px 8px 8px;
    padding: 0px;
    margin-top: 0px;
}}

QTabBar::tab {{
    background-color: {COLORS['bg_tertiary']};
    color: {COLORS['text_muted']};
    border: none;
    padding: 12px 28px;
    margin-right: 0px;
    border-radius: 0px;
    font-weight: 600;
    font-size: 10pt;
}}

QTabBar::tab:first {{
    border-radius: 8px 0 0 0;
}}

QTabBar::tab:last {{
    border-radius: 0 8px 0 0;
}}

QTabBar::tab:selected {{
    background-color: {COLORS['bg_secondary']};
    color: {COLORS['text_primary']};
}}

QTabBar::tab:hover:!selected {{
    background-color: {COLORS['bg_hover']};
    color: {COLORS['text_secondary']};
}}

/* Sub-tabs (Processes/Resources/Edges) */
QTabWidget[objectName="subTabs"]::pane {{
    border: none;
    background-color: transparent;
    padding: 4px 0;
}}

QTabWidget[objectName="subTabs"] > QTabBar::tab {{
    background-color: transparent;
    color: {COLORS['text_muted']};
    padding: 8px 16px;
    margin-right: 4px;
    border-radius: 6px;
    font-weight: 500;
    font-size: 9pt;
}}

QTabWidget[objectName="subTabs"] > QTabBar::tab:selected {{
    background-color: {COLORS['primary']};
    color: {COLORS['text_white']};
}}

QTabWidget[objectName="subTabs"] > QTabBar::tab:hover:!selected {{
    background-color: {COLORS['bg_hover']};
}}

/* Table Widget / Tree Widget - Clean table style */
QTableWidget, QTreeWidget, QTableView, QTreeView {{
    background-color: {COLORS['bg_input']};
    alternate-background-color: {COLORS['bg_secondary']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 8px;
    gridline-color: {COLORS['border']};
    selection-background-color: {COLORS['primary']};
}}

QTableWidget::item, QTreeWidget::item {{
    padding: 8px 12px;
    border-bottom: 1px solid {COLORS['border']};
}}

QTableWidget::item:selected, QTreeWidget::item:selected {{
    background-color: {COLORS['primary']};
    color: {COLORS['text_white']};
}}

QTableWidget::item:hover, QTreeWidget::item:hover {{
    background-color: {COLORS['bg_hover']};
}}

QHeaderView::section {{
    background-color: {COLORS['bg_tertiary']};
    color: {COLORS['text_secondary']};
    padding: 10px 12px;
    border: none;
    border-bottom: 2px solid {COLORS['border']};
    font-weight: 600;
    text-transform: uppercase;
    font-size: {FONTS['size_small']}pt;
}}

/* Scroll Bar - Minimal style */
QScrollBar:vertical {{
    background-color: transparent;
    width: 10px;
    border-radius: 5px;
    margin: 0;
}}

QScrollBar::handle:vertical {{
    background-color: {COLORS['border_light']};
    min-height: 30px;
    border-radius: 5px;
    margin: 2px;
}}

QScrollBar::handle:vertical:hover {{
    background-color: {COLORS['primary']};
}}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
    height: 0;
}}

QScrollBar:horizontal {{
    background-color: transparent;
    height: 10px;
    border-radius: 5px;
}}

QScrollBar::handle:horizontal {{
    background-color: {COLORS['border_light']};
    min-width: 30px;
    border-radius: 5px;
    margin: 2px;
}}

QScrollBar::handle:horizontal:hover {{
    background-color: {COLORS['primary']};
}}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {{
    width: 0;
}}

/* Check Box */
QCheckBox {{
    color: {COLORS['text_primary']};
    spacing: 10px;
}}

QCheckBox::indicator {{
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 2px solid {COLORS['border_light']};
    background-color: {COLORS['bg_input']};
}}

QCheckBox::indicator:checked {{
    background-color: {COLORS['primary']};
    border-color: {COLORS['primary']};
}}

QCheckBox::indicator:hover {{
    border-color: {COLORS['primary_light']};
}}

/* Radio Button */
QRadioButton {{
    color: {COLORS['text_primary']};
    spacing: 10px;
}}

QRadioButton::indicator {{
    width: 18px;
    height: 18px;
    border-radius: 9px;
    border: 2px solid {COLORS['border_light']};
    background-color: {COLORS['bg_input']};
}}

QRadioButton::indicator:checked {{
    background-color: {COLORS['primary']};
    border-color: {COLORS['primary']};
}}

/* Progress Bar */
QProgressBar {{
    background-color: {COLORS['bg_tertiary']};
    border-radius: 6px;
    text-align: center;
    color: {COLORS['text_white']};
    height: 20px;
    border: none;
}}

QProgressBar::chunk {{
    background-color: {COLORS['primary']};
    border-radius: 6px;
}}

/* Splitter */
QSplitter::handle {{
    background-color: {COLORS['border']};
}}

QSplitter::handle:horizontal {{
    width: 2px;
}}

QSplitter::handle:vertical {{
    height: 2px;
}}

QSplitter::handle:hover {{
    background-color: {COLORS['primary']};
}}

/* Menu Bar */
QMenuBar {{
    background-color: {COLORS['bg_secondary']};
    color: {COLORS['text_primary']};
    border-bottom: 1px solid {COLORS['border']};
    padding: 4px 8px;
}}

QMenuBar::item {{
    padding: 8px 14px;
    border-radius: 6px;
}}

QMenuBar::item:selected {{
    background-color: {COLORS['bg_hover']};
}}

/* Menu */
QMenu {{
    background-color: {COLORS['bg_secondary']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 8px;
    padding: 6px;
}}

QMenu::item {{
    padding: 10px 24px;
    border-radius: 6px;
}}

QMenu::item:selected {{
    background-color: {COLORS['primary']};
}}

QMenu::separator {{
    height: 1px;
    background-color: {COLORS['border']};
    margin: 6px 12px;
}}

/* Status Bar */
QStatusBar {{
    background-color: {COLORS['bg_secondary']};
    color: {COLORS['text_secondary']};
    border-top: 1px solid {COLORS['border']};
    padding: 4px 8px;
}}

/* Tool Tip */
QToolTip {{
    background-color: {COLORS['bg_tertiary']};
    color: {COLORS['text_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 6px;
    padding: 8px 12px;
}}

/* Graphics View (for RAG) */
QGraphicsView {{
    background-color: {COLORS['bg_primary']};
    border: 1px solid {COLORS['border']};
    border-radius: 8px;
}}

/* Slider */
QSlider::groove:horizontal {{
    border: none;
    height: 6px;
    background-color: {COLORS['bg_tertiary']};
    border-radius: 3px;
}}

QSlider::handle:horizontal {{
    background-color: {COLORS['primary']};
    width: 16px;
    height: 16px;
    margin: -5px 0;
    border-radius: 8px;
}}

QSlider::handle:horizontal:hover {{
    background-color: {COLORS['primary_light']};
}}

QSlider::sub-page:horizontal {{
    background-color: {COLORS['primary']};
    border-radius: 3px;
}}
"""


def apply_theme(app: QApplication):
    """Apply the dark theme to the application"""
    app.setStyleSheet(STYLESHEET)
    
    # Set the application palette for consistency
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(COLORS['bg_primary']))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(COLORS['text_primary']))
    palette.setColor(QPalette.ColorRole.Base, QColor(COLORS['bg_secondary']))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(COLORS['bg_tertiary']))
    palette.setColor(QPalette.ColorRole.Text, QColor(COLORS['text_primary']))
    palette.setColor(QPalette.ColorRole.Button, QColor(COLORS['primary']))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(COLORS['text_white']))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(COLORS['primary']))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(COLORS['text_white']))
    
    app.setPalette(palette)
