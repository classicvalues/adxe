/****************************************************************************
 Copyright (c) 2015 Mazyad Alabduljaleel
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 https://adxeproject.github.io/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#import "ui/UIEditBox/iOS/UITextView+CCUITextInput.h"

@implementation UITextView (CCUITextInput)

- (NSString*)ccui_text
{
    return self.text;
}

- (void)ccui_setText:(NSString*)ccui_text
{
    self.text = ccui_text;
}

- (NSString*)ccui_placeholder
{
    SEL selector = @selector(placeholder);
    if ([self respondsToSelector:selector])
    {
        return [self performSelector:selector];
    }
    return nil;
}

- (void)ccui_setPlaceholder:(NSString*)ccui_placeholder
{
    SEL selector = @selector(setPlaceholder:);
    if ([self respondsToSelector:selector])
    {
        [self performSelector:selector withObject:ccui_placeholder];
    }
}

- (UIColor*)ccui_textColor
{
    return self.textColor;
}

- (void)ccui_setTextColor:(UIColor*)ccui_textColor
{
    self.textColor = ccui_textColor;
}

- (UIFont*)ccui_font
{
    return self.font;
}

- (void)ccui_setFont:(UIFont*)ccui_font
{
    self.font = ccui_font;
}

- (NSTextAlignment)ccui_alignment
{
    return self.textAlignment;
}

- (void)ccui_setTextHorizontalAlignment:(NSTextAlignment)ccui_alignment
{
    self.textAlignment = ccui_alignment;
}

- (UIColor*)ccui_placeholderTextColor
{
    SEL selector = NSSelectorFromString(@"placeHolderLabel");
    if ([self respondsToSelector:selector])
    {
        return ((UILabel*)[self performSelector:selector]).textColor;
    }
    return nil;
}

- (void)ccui_setPlaceholderTextColor:(UIColor*)ccui_placeholderTextColor
{
    SEL selector = NSSelectorFromString(@"placeHolderLabel");
    if ([self respondsToSelector:selector])
    {
        ((UILabel*)[self performSelector:selector]).textColor = ccui_placeholderTextColor;
    }
}

- (UIFont*)ccui_placeholderFont
{
    SEL selector = NSSelectorFromString(@"placeHolderLabel");
    if ([self respondsToSelector:selector])
    {
        return ((UILabel*)[self performSelector:selector]).font;
    }
    return nil;
}

- (void)ccui_setPlaceholderFont:(UIFont*)ccui_placeholderFont
{
    SEL selector = NSSelectorFromString(@"placeHolderLabel");
    if ([self respondsToSelector:selector])
    {
        ((UILabel*)[self performSelector:selector]).font = ccui_placeholderFont;
    }
}

- (BOOL)ccui_secureTextEntry
{
    return self.secureTextEntry;
}

- (void)ccui_setSecureTextEntry:(BOOL)ccui_secureTextEntry
{
    self.secureTextEntry = ccui_secureTextEntry;
}

- (void)ccui_setDelegate:(id<UITextFieldDelegate, UITextViewDelegate>)delegate
{
    self.delegate = delegate;
}

@end

void LoadUITextViewCCUITextInputCategory() {}
