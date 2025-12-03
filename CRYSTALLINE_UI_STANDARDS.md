# Crystalline UI - Usability Standards

## Purpose
This document defines the usability standards for the Crystalline UI system to ensure all interfaces are **logical, comfortable, and accessible for human users** while maintaining sacred geometry principles.

## Core Principle
**Usability First, Sacred Geometry Second**

While we incorporate sacred geometry and crystalline mathematics, the interface must be:
1. Easy to use
2. Comfortable to interact with
3. Accessible to all users
4. Intuitive and logical

## Button Size Standards

### Minimum Sizes (Never Go Below)
- **Minimum radius**: 40px (80px diameter)
- **Reason**: Accessibility and comfortable clicking
- **Applies to**: All clickable buttons

### Recommended Sizes by Type

#### Primary Actions (Start, Stop, Submit, Confirm)
- **Radius**: 60-80px (120-160px diameter)
- **Use Case**: Main action buttons that users click frequently
- **Example**: Video tab START button
- **Spacing**: 100-120px between buttons

#### Secondary Actions (Add, Remove, Delete, Save)
- **Radius**: 50px (100px diameter)
- **Use Case**: Common actions users perform regularly
- **Example**: URL Manager ADD/REMOVE buttons
- **Spacing**: 80-100px between buttons

#### Tertiary Actions (Sort, Filter, Options, Settings)
- **Radius**: 40-45px (80-90px diameter)
- **Use Case**: Utility actions and less frequent operations
- **Example**: Research tab sort buttons
- **Spacing**: 60-80px between buttons

### Button Spacing
- **Minimum spacing**: 10px between button edges
- **Recommended spacing**: 20px between button edges
- **Vertical stacking**: 60-80px center-to-center
- **Horizontal arrangement**: 80-100px center-to-center

## Input Field Standards

### Height
- **Minimum height**: 40px
- **Recommended height**: 44-48px
- **Reason**: Comfortable typing and clicking

### Width
- **Minimum width**: 200px
- **Recommended width**: 300-400px for text input
- **Search fields**: 250-350px
- **URL fields**: 400-500px

### Style Considerations
- **Border**: Clear, visible border (2-3px)
- **Padding**: 10-15px internal padding
- **Font size**: 14-16px
- **Placeholder text**: 14px, slightly dimmed

### Shape Recommendations
- **Text input**: Rectangular with rounded corners (8-12px radius)
- **Search input**: Rounded rectangle or pill shape
- **Dodecagon border**: Use for decoration only, not for input field shape

## List Standards

### Line Height
- **Minimum**: 20px
- **Recommended**: 24-28px
- **Reason**: Comfortable reading and selection

### Item Spacing
- **Padding**: 8-12px vertical, 10-15px horizontal
- **Hover state**: Clear visual feedback
- **Selected state**: Distinct background color

### Font Size
- **List items**: 14-16px
- **Headers**: 16-18px bold
- **Metadata**: 12-14px (slightly dimmed)

## Panel Standards

### Minimum Sizes
- **Width**: 300px minimum
- **Height**: 200px minimum
- **Padding**: 20px internal padding

### Title/Header
- **Font size**: 18-20px bold
- **Spacing**: 15-20px below header
- **Alignment**: Center or left-aligned

### Content Area
- **Padding**: 15-20px from panel edges
- **Spacing**: 15-20px between elements
- **Overflow**: Scrollable if content exceeds panel

## Text Standards

### Font Sizes
- **Headers (H1)**: 24-28px
- **Subheaders (H2)**: 20-24px
- **Section titles (H3)**: 18-20px
- **Body text**: 14-16px
- **Small text**: 12-14px
- **Minimum readable**: 12px (use sparingly)

### Line Height
- **Body text**: 1.5x font size (e.g., 16px font = 24px line height)
- **Headers**: 1.2-1.3x font size
- **Lists**: 1.6-1.8x font size

### Text Color Contrast
- **Minimum contrast ratio**: 4.5:1 (WCAG AA)
- **Recommended ratio**: 7:1 (WCAG AAA)
- **Test with**: Color contrast checker tools

## Color Standards

### Sacred Frequency Colors (Use with Care)
- **432 Hz**: Primary text - MUST have good contrast
- **528 Hz**: Content/transformation - MUST be readable
- **639 Hz**: Accent/connection - Use for highlights
- **741 Hz**: Warning/attention - Use sparingly

### Contrast Requirements
1. **Text on background**: Minimum 4.5:1 ratio
2. **Large text (18px+)**: Minimum 3:1 ratio
3. **Interactive elements**: Minimum 3:1 ratio
4. **Test with**: WCAG contrast checker

### Color Blindness Considerations
- **Don't rely on color alone**: Use icons, text, or patterns
- **Test with**: Color blind simulators
- **Common types**: Red-green, blue-yellow

## Layout Standards

### Golden Ratio Usage
- **Use for**: Major layout divisions
- **Example**: 61.8% / 38.2% split
- **Don't overuse**: Not every element needs golden ratio

### Spacing System
- **Base unit**: 8px
- **Small spacing**: 8px
- **Medium spacing**: 16px
- **Large spacing**: 24px
- **Extra large**: 32px

### Grid System
- **Column width**: Flexible based on content
- **Gutter**: 16-24px between columns
- **Margins**: 20-40px from screen edges

## Accessibility Standards

### Keyboard Navigation
- **Tab order**: Logical and intuitive
- **Focus indicators**: Clear and visible (2-3px outline)
- **Shortcuts**: Document all keyboard shortcuts
- **Escape key**: Always allows exit from modals/dialogs

### Screen Reader Support
- **Button labels**: Clear and descriptive
- **ARIA labels**: Use for complex elements
- **Alt text**: Provide for all images
- **Semantic HTML**: Use proper heading hierarchy

### Touch Targets (for future touch support)
- **Minimum size**: 44x44px (Apple HIG)
- **Recommended**: 48x48px (Material Design)
- **Spacing**: 8px minimum between targets

## Animation Standards

### Timing
- **Fast**: 150-200ms (hover effects)
- **Medium**: 250-350ms (transitions)
- **Slow**: 400-600ms (major changes)
- **Never**: Over 1 second

### Easing
- **Ease-out**: For appearing elements
- **Ease-in**: For disappearing elements
- **Ease-in-out**: For moving elements

### Respect User Preferences
- **Reduced motion**: Provide option to disable animations
- **System settings**: Respect OS-level motion preferences

## Responsive Design

### Breakpoints
- **Small**: < 768px (tablets portrait)
- **Medium**: 768-1024px (tablets landscape)
- **Large**: 1024-1440px (laptops)
- **Extra large**: > 1440px (desktops)

### Scaling
- **Buttons**: Scale proportionally
- **Text**: Adjust font sizes for readability
- **Panels**: Adjust widths, maintain minimum sizes

## Testing Checklist

### Manual Testing
- [ ] Can you easily click all buttons?
- [ ] Can you easily read all text?
- [ ] Can you easily select items in lists?
- [ ] Can you easily type in input fields?
- [ ] Is the layout intuitive?
- [ ] Do you understand what each button does?
- [ ] Can you navigate with keyboard only?
- [ ] Are error messages clear?

### Automated Testing
- [ ] Contrast ratios meet WCAG AA
- [ ] Button sizes meet minimum standards
- [ ] Input fields meet minimum heights
- [ ] Line heights meet minimum standards
- [ ] Font sizes meet minimum standards

### User Testing
- [ ] Time to complete common tasks
- [ ] Error rate (mis-clicks)
- [ ] User satisfaction rating
- [ ] Accessibility compliance

## Implementation Checklist

When creating a new tab, verify:

### Buttons
- [ ] All buttons minimum 40px radius (80px diameter)
- [ ] Primary actions 60-80px radius
- [ ] Secondary actions 50px radius
- [ ] Tertiary actions 40-45px radius
- [ ] Proper spacing between buttons

### Input Fields
- [ ] Minimum height 40px
- [ ] Proper width for content type
- [ ] Clear borders and padding
- [ ] Readable font size (14-16px)

### Lists
- [ ] Line height minimum 20px
- [ ] Proper item padding
- [ ] Clear hover/selection states
- [ ] Readable font size (14-16px)

### Text
- [ ] Body text 14-16px
- [ ] Headers appropriately sized
- [ ] Line height 1.5x font size
- [ ] Good contrast ratios

### Layout
- [ ] Logical arrangement
- [ ] Proper spacing
- [ ] Responsive to window size
- [ ] No overlapping elements

### Accessibility
- [ ] Keyboard navigation works
- [ ] Focus indicators visible
- [ ] Screen reader compatible
- [ ] Color blind friendly

## Sacred Geometry Integration

### How to Balance Beauty and Usability

1. **Use sacred geometry for layout proportions**
   - Golden ratio for major divisions
   - 12-fold symmetry for arrangements
   - Fibonacci spacing where appropriate

2. **Use circular buttons for actions**
   - But make them large enough to click
   - Minimum 40px radius

3. **Use frequency-based colors**
   - But ensure good contrast
   - Test with contrast checkers

4. **Use dodecagon borders for decoration**
   - Not for input field shapes
   - Use as panel borders or accents

5. **Use clock lattice for complex arrangements**
   - But maintain logical grouping
   - Don't sacrifice usability for symmetry

## Examples of Good Balance

### Video Tab
- ✅ Large central button (80px radius)
- ✅ Clear layout with panels
- ✅ Good spacing
- ✅ Readable text

### URL Manager (After Fixes)
- ✅ Buttons increased to 50px radius
- ✅ Input field increased to 40px height
- ✅ Clear three-column layout
- ✅ Logical grouping

### Research Tab (After Fixes)
- ✅ Buttons increased to 40-45px radius
- ✅ Input field increased to 40px height
- ✅ Golden ratio layout
- ✅ Clear file browser interface

## Conclusion

**Remember**: Users don't care about sacred geometry if they can't use the interface. Make it usable first, beautiful second. The best design is one that users don't notice because it just works.

---

**Date**: December 3, 2024
**Version**: 1.0
**Status**: Active Standard
**Review**: Before each new tab implementation