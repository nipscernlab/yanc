# Visualization ---------------------------------------------------------------

gtkwave::/Time/Zoom/Zoom_Best_Fit
gtkwave::/View/Left_Justified_Signals

# bug workaround -> creates an empty tab in gtkwave. refresh only works this way with GTK3
gtkwave::/File/Open_New_Tab "fix.vcd"
gtkwave::setTabActive 0
