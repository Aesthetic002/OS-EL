"""
Theme and Styling for OS-EL Deadlock Detection GUI

Provides a modern, professional color palette and styling utilities.
"""

# Color Palette
COLORS = {
    # Primary colors
    'primary': '#2563eb',      # Blue
    'primary_dark': '#1e40af',
    'primary_light': '#60a5fa',
    
    # Secondary colors
    'secondary': '#64748b',    # Slate
    'secondary_dark': '#475569',
    'secondary_light': '#94a3b8',
    
    # Accent colors
    'accent': '#8b5cf6',       # Purple
    'success': '#10b981',      # Green
    'warning': '#f59e0b',      # Amber
    'error': '#ef4444',        # Red
    'info': '#06b6d4',         # Cyan
    
    # Background colors
    'bg_primary': '#ffffff',
    'bg_secondary': '#f8fafc',
    'bg_tertiary': '#e2e8f0',
    
    # Text colors
    'text_primary': '#0f172a',
    'text_secondary': '#475569',
    'text_muted': '#94a3b8',
    'text_white': '#ffffff',
    
    # Border colors
    'border': '#cbd5e1',
    'border_light': '#e2e8f0',
    'border_dark': '#94a3b8',
    
    # State colors
    'process': '#3b82f6',      # Blue for processes
    'resource': '#8b5cf6',     # Purple for resources
    'deadlock': '#ef4444',     # Red for deadlocked items
    'safe': '#10b981',         # Green for safe state
    
    # Graph colors
    'node_process': '#3b82f6',
    'node_resource': '#8b5cf6',
    'edge_request': '#f59e0b',
    'edge_assignment': '#10b981',
}

# Font Configurations
FONTS = {
    'header_large': ('Segoe UI', 16, 'bold'),
    'header': ('Segoe UI', 12, 'bold'),
    'header_small': ('Segoe UI', 10, 'bold'),
    'body': ('Segoe UI', 10),
    'body_small': ('Segoe UI', 9),
    'mono': ('Consolas', 9),
    'mono_large': ('Consolas', 10),
}

# Widget Styles
BUTTON_STYLE = {
    'font': FONTS['body'],
    'bg': COLORS['primary'],
    'fg': COLORS['text_white'],
    'activebackground': COLORS['primary_dark'],
    'relief': 'flat',
    'padx': 12,
    'pady': 6,
    'cursor': 'hand2',
}

BUTTON_SECONDARY_STYLE = {
    'font': FONTS['body'],
    'bg': COLORS['secondary'],
    'fg': COLORS['text_white'],
    'activebackground': COLORS['secondary_dark'],
    'relief': 'flat',
    'padx': 12,
    'pady': 6,
    'cursor': 'hand2',
}

BUTTON_DANGER_STYLE = {
    'font': FONTS['body'],
    'bg': COLORS['error'],
    'fg': COLORS['text_white'],
    'activebackground': '#dc2626',
    'relief': 'flat',
    'padx': 12,
    'pady': 6,
    'cursor': 'hand2',
}

LABEL_STYLE = {
    'font': FONTS['body'],
    'bg': COLORS['bg_primary'],
    'fg': COLORS['text_primary'],
}

LABEL_HEADER_STYLE = {
    'font': FONTS['header'],
    'bg': COLORS['bg_primary'],
    'fg': COLORS['text_primary'],
}

ENTRY_STYLE = {
    'font': FONTS['body'],
    'bg': COLORS['bg_primary'],
    'fg': COLORS['text_primary'],
    'relief': 'solid',
    'borderwidth': 1,
}

FRAME_STYLE = {
    'bg': COLORS['bg_primary'],
}

FRAME_SECONDARY_STYLE = {
    'bg': COLORS['bg_secondary'],
}

TEXT_STYLE = {
    'font': FONTS['mono'],
    'bg': COLORS['bg_primary'],
    'fg': COLORS['text_primary'],
    'relief': 'solid',
    'borderwidth': 1,
    'wrap': 'word',
}


def apply_button_style(button, style='primary'):
    """Apply style to a button"""
    if style == 'primary':
        button.config(**BUTTON_STYLE)
    elif style == 'secondary':
        button.config(**BUTTON_SECONDARY_STYLE)
    elif style == 'danger':
        button.config(**BUTTON_DANGER_STYLE)
    
    # Add hover effects
    def on_enter(e):
        if style == 'primary':
            button.config(bg=COLORS['primary_dark'])
        elif style == 'secondary':
            button.config(bg=COLORS['secondary_dark'])
        elif style == 'danger':
            button.config(bg='#dc2626')
    
    def on_leave(e):
        if style == 'primary':
            button.config(bg=COLORS['primary'])
        elif style == 'secondary':
            button.config(bg=COLORS['secondary'])
        elif style == 'danger':
            button.config(bg=COLORS['error'])
    
    button.bind('<Enter>', on_enter)
    button.bind('<Leave>', on_leave)


def apply_label_style(label, header=False):
    """Apply style to a label"""
    if header:
        label.config(**LABEL_HEADER_STYLE)
    else:
        label.config(**LABEL_STYLE)


def apply_entry_style(entry):
    """Apply style to an entry"""
    entry.config(**ENTRY_STYLE)


def apply_frame_style(frame, secondary=False):
    """Apply style to a frame"""
    if secondary:
        frame.config(**FRAME_SECONDARY_STYLE)
    else:
        frame.config(**FRAME_STYLE)


def apply_text_style(text_widget):
    """Apply style to a text widget"""
    text_widget.config(**TEXT_STYLE)
