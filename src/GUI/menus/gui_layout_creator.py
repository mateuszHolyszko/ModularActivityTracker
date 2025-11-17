import tkinter as tk
from tkinter import simpledialog, filedialog, messagebox
import re
import os

GRID_SIZE = 10
CANVAS_WIDTH = 800
CANVAS_HEIGHT = 480


class BoxDesigner:
    def __init__(self, root):
        self.root = root
        self.root.title("GUI Layout Designer")

        # Canvas setup
        self.canvas = tk.Canvas(root, width=CANVAS_WIDTH, height=CANVAS_HEIGHT, bg="white")
        self.canvas.pack()

        # Draw grid
        for i in range(0, CANVAS_WIDTH, GRID_SIZE):
            self.canvas.create_line([(i, 0), (i, CANVAS_HEIGHT)], fill="#eee", tags="grid_line")
        for i in range(0, CANVAS_HEIGHT, GRID_SIZE):
            self.canvas.create_line([(0, i), (CANVAS_WIDTH, i)], fill="#eee", tags="grid_line")

        self.boxes = []  # list of {id, text_id, x, y, width, height, label}
        self.start_x = None
        self.start_y = None
        self.current_rect = None
        self.size_text_id = None

        self.resizing_box = None
        self.resize_corner = None
        self.resize_text_id = None  # NEW: Text for resize dimensions

        # For moving existing boxes
        self.dragging_box = None
        self.drag_offset_x = 0
        self.drag_offset_y = 0

        # Buttons
        toolbar = tk.Frame(root)
        toolbar.pack(pady=5)
        tk.Button(toolbar, text="Export to .hpp", command=self.export_to_hpp).pack(side=tk.LEFT, padx=4)
        tk.Button(toolbar, text="Load .hpp", command=self.load_from_hpp).pack(side=tk.LEFT, padx=4)

        # Event bindings
        self.canvas.bind("<Button-1>", self.on_click)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_release)
        self.canvas.bind("<Button-3>", self.on_right_click)

    def snap(self, value):
        """Snap value to nearest GRID_SIZE multiple"""
        return round(value / GRID_SIZE) * GRID_SIZE

    def find_box_by_id(self, cid):
        for box in self.boxes:
            if box["id"] == cid or box["text_id"] == cid:
                return box
        return None

    def on_click(self, event):
        clicked = self.canvas.find_closest(event.x, event.y)
        box = self.find_box_by_id(clicked[0])

        if box:
            # Check if near a corner → resize
            corner = self.get_corner_if_near(box, event.x, event.y)
            if corner:
                self.resizing_box = box
                self.resize_corner = corner
                # NEW: Create resize dimension text
                self.resize_text_id = self.canvas.create_text(
                    event.x, event.y - 15, text="", fill="red", anchor="w"
                )
                return

            # Otherwise move box
            self.dragging_box = box
            self.drag_offset_x = event.x - box["x"]
            self.drag_offset_y = event.y - box["y"]
        else:
            # Start drawing new box
            self.start_x = self.snap(event.x)
            self.start_y = self.snap(event.y)
            self.current_rect = self.canvas.create_rectangle(
                self.start_x, self.start_y, self.start_x, self.start_y, outline="blue", width=2
            )
            self.size_text_id = self.canvas.create_text(
                self.start_x + 5, self.start_y - 10, text="", fill="gray", anchor="w"
            )

    def on_drag(self, event):
        if self.resizing_box:
            box = self.resizing_box
            x, y = self.snap(event.x), self.snap(event.y)

            # NEW: Update dimension text position and content
            if self.resize_text_id:
                width = box["width"]
                height = box["height"]
                
                # Calculate new dimensions based on corner
                if self.resize_corner == "nw":
                    new_w = box["x"] + box["width"] - x
                    new_h = box["y"] + box["height"] - y
                elif self.resize_corner == "ne":
                    new_w = x - box["x"]
                    new_h = box["y"] + box["height"] - y
                elif self.resize_corner == "sw":
                    new_w = box["x"] + box["width"] - x
                    new_h = y - box["y"]
                elif self.resize_corner == "se":
                    new_w = x - box["x"]
                    new_h = y - box["y"]
                
                self.canvas.itemconfig(self.resize_text_id, text=f"{new_w}x{new_h}")
                self.canvas.coords(self.resize_text_id, event.x + 10, event.y - 15)

            # Resize depending on corner
            if self.resize_corner == "nw":
                new_x, new_y = x, y
                new_w = box["x"] + box["width"] - new_x
                new_h = box["y"] + box["height"] - new_y
                box["x"], box["y"], box["width"], box["height"] = new_x, new_y, new_w, new_h
            elif self.resize_corner == "ne":
                new_w = x - box["x"]
                new_h = box["y"] + box["height"] - y
                box["y"] = y
                box["width"], box["height"] = new_w, new_h
            elif self.resize_corner == "sw":
                new_w = box["x"] + box["width"] - x
                new_h = y - box["y"]
                box["x"] = x
                box["width"], box["height"] = new_w, new_h
            elif self.resize_corner == "se":
                box["width"] = x - box["x"]
                box["height"] = y - box["y"]

            # Update rectangle and label
            self.canvas.coords(box["id"], box["x"], box["y"], box["x"] + box["width"], box["y"] + box["height"])
            self.canvas.coords(box["text_id"], box["x"] + box["width"] / 2, box["y"] + box["height"] / 2)
            return

        # Existing drag code for moving boxes or drawing new ones
        if self.dragging_box:
            new_x = self.snap(event.x - self.drag_offset_x)
            new_y = self.snap(event.y - self.drag_offset_y)
            box = self.dragging_box
            box["x"], box["y"] = new_x, new_y
            self.canvas.coords(box["id"], new_x, new_y, new_x + box["width"], new_y + box["height"])
            self.canvas.coords(box["text_id"], new_x + box["width"] / 2, new_y + box["height"] / 2)
            return

        if self.current_rect:
            cur_x, cur_y = self.snap(event.x), self.snap(event.y)
            self.canvas.coords(self.current_rect, self.start_x, self.start_y, cur_x, cur_y)
            width = abs(cur_x - self.start_x)
            height = abs(cur_y - self.start_y)
            self.canvas.itemconfig(self.size_text_id, text=f"{width}x{height}")
            self.canvas.coords(self.size_text_id, cur_x + 5, cur_y - 10)

    def on_release(self, event):
        # NEW: Clean up resize dimension text
        if self.resizing_box:
            if self.resize_text_id:
                self.canvas.delete(self.resize_text_id)
                self.resize_text_id = None
            self.resizing_box = None
            self.resize_corner = None
            return

        if self.dragging_box:
            self.dragging_box = None
            return

        if not self.current_rect:
            return

        x1, y1, x2, y2 = self.canvas.coords(self.current_rect)
        width = abs(x2 - x1)
        height = abs(y2 - y1)

        if width < 5 or height < 5:
            self.canvas.delete(self.current_rect)
            self.canvas.delete(self.size_text_id)
            return

        label = simpledialog.askstring("Box Label", "Enter label:")
        if not label:
            self.canvas.delete(self.current_rect)
            self.canvas.delete(self.size_text_id)
            return

        text_x = (x1 + x2) / 2
        text_y = (y1 + y2) / 2
        text_id = self.canvas.create_text(text_x, text_y, text=label, fill="black")

        self.boxes.append({
            "id": self.current_rect,
            "text_id": text_id,
            "x": int(min(x1, x2)),
            "y": int(min(y1, y2)),
            "width": int(width),
            "height": int(height),
            "label": label
        })

        self.canvas.delete(self.size_text_id)
        self.size_text_id = None
        self.current_rect = None

    def on_right_click(self, event):
        """Delete box on right-click"""
        clicked = self.canvas.find_closest(event.x, event.y)
        box = self.find_box_by_id(clicked[0])
        if box:
            self.canvas.delete(box["id"])
            self.canvas.delete(box["text_id"])
            self.boxes.remove(box)

    def get_corner_if_near(self, box, x, y, threshold=10):
        corners = {
            "nw": (box["x"], box["y"]),
            "ne": (box["x"] + box["width"], box["y"]),
            "sw": (box["x"], box["y"] + box["height"]),
            "se": (box["x"] + box["width"], box["y"] + box["height"])
        }
        for name, (cx, cy) in corners.items():
            if abs(cx - x) <= threshold and abs(cy - y) <= threshold:
                return name
        return None

    def export_to_hpp(self):
        if not self.boxes:
            messagebox.showwarning("No boxes", "Nothing to export.")
            return

        filepath = filedialog.asksaveasfilename(defaultextension=".hpp", filetypes=[("C++ Header", "*.hpp")])
        if not filepath:
            return

        with open(filepath, "w") as f:
            f.write("// Auto-generated layout (dictionary-style)\n")
            f.write("#pragma once\n")
            f.write("#include <unordered_map>\n#include <string>\n\n")
            f.write("struct Box { int x, y, width, height; };\n\n")
            f.write("static const std::unordered_map<std::string, Box> layout = {\n")

            for box in self.boxes:
                key = box['label']
                f.write(f'    {{"{key}", {{ {box["x"]}, {box["y"]}, {box["width"]}, {box["height"]} }} }},\n')

            f.write("};\n")

        messagebox.showinfo("Exported", f"Layout saved to:\n{filepath}")

    def load_from_hpp(self):
        filepath = filedialog.askopenfilename(filetypes=[("C++ Header", "*.hpp")])
        if not filepath:
            return

        with open(filepath, "r") as f:
            content = f.read()

        # New regex for dictionary-style map: "label", {x, y, w, h}
        pattern = re.compile(r'"\s*([^"]+)\s*"\s*,\s*\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\}')
        matches = pattern.findall(content)

        if not matches:
            messagebox.showwarning("No layout data", "No boxes found in file.")
            return

        # Clear canvas
        for box in list(self.boxes):
            self.canvas.delete(box["id"])
            self.canvas.delete(box["text_id"])
        self.boxes.clear()

        # Recreate boxes
        for match in matches:
            label, x, y, w, h = match
            x, y, w, h = map(int, (x, y, w, h))
            rect_id = self.canvas.create_rectangle(x, y, x + w, y + h, outline="blue", width=2)
            text_id = self.canvas.create_text(x + w / 2, y + h / 2, text=label, fill="black")
            self.boxes.append({
                "id": rect_id,
                "text_id": text_id,
                "x": x,
                "y": y,
                "width": w,
                "height": h,
                "label": label
            })

        messagebox.showinfo("Loaded", f"Loaded {len(self.boxes)} boxes from:\n{os.path.basename(filepath)}")


if __name__ == "__main__":
    root = tk.Tk()
    app = BoxDesigner(root)
    root.mainloop()