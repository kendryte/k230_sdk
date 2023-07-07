
# SCons PreProcessor patch
def start_handling_includes(self, t=None):
    """
    Causes the PreProcessor object to start processing #import,
    #include and #include_next lines.

    This method will be called when a #if, #ifdef, #ifndef or #elif
    evaluates True, or when we reach the #else in a #if, #ifdef,
    #ifndef or #elif block where a condition already evaluated
    False.

    """
    d = self.dispatch_table
    p = self.stack[-1] if self.stack else self.default_table

    for k in ('import', 'include', 'include_next', 'define'):
        d[k] = p[k]

def stop_handling_includes(self, t=None):
    """
    Causes the PreProcessor object to stop processing #import,
    #include and #include_next lines.

    This method will be called when a #if, #ifdef, #ifndef or #elif
    evaluates False, or when we reach the #else in a #if, #ifdef,
    #ifndef or #elif block where a condition already evaluated True.
    """
    d = self.dispatch_table
    d['import'] = self.do_nothing
    d['include'] =  self.do_nothing
    d['include_next'] =  self.do_nothing
    d['define'] =  self.do_nothing

PatchedPreProcessor = SCons.cpp.PreProcessor
PatchedPreProcessor.start_handling_includes = start_handling_includes
PatchedPreProcessor.stop_handling_includes = stop_handling_includes
