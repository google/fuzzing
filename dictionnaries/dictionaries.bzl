"""Build extensions for //security/fuzzing/dictionaries."""

def basename(filename):
    """
    Returns the basename of `filename`.

    Args:
        filename: the filename to return the basename of.
    Returns:
        the basename of the `filename` parameter
    """
    dirparts = filename.split("/")
    fullname = dirparts.pop()
    if not fullname:  # If `filename` is ending with `/`
        fullname = dirparts.pop()

    parts = fullname.split(".")
    if len(parts) in [0, 1]:  # No Extension
        return filename
    else:
        parts.pop()  # Delete the last item.
        return "".join(parts)

def run():
    for f in native.glob(["*.dict"]):
        native.filegroup(
            name = basename(f),
            srcs = [f],
        )
