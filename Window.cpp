/*
 * Copyright 2010 Daniel Albano
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Window.h"

/**
 * Sets up a blank window with the specified size and position.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 * @param popup Popup animation.
 */
Window::Window(int width, int height, int x, int y, WindowPopup popup) : Surface(width, height, x, y), _bg(0), _color(0), _popup(popup), _popupStep(0.0)
{
	if (_popup == POPUP_NONE)
		_popupStep = 1.0;
}

/**
 *
 */
Window::~Window()
{
	
}

/**
 * Changes the surface used to draw the background of the window.
 * @param bg New background.
 */
void Window::setBg(Surface *bg)
{
	_bg = bg;
	draw();
}

/**
 * Changes the color used to draw the shaded border.
 * @param color Color value.
 */
void Window::setColor(Uint8 color)
{
	_color = color;
	draw();
}

/**
 * Returns the color used to draw the shaded border.
 * @return Color value.
 */
Uint8 Window::getColor()
{
	return _color;
}

/**
 * Draws the bordered window with a graphic background.
 * The background never moves with the window, it's
 * always aligned to the top-left corner of the screen
 * and cropped to fit the inside area.
 */
void Window::draw()
{
	SDL_Rect cropper;
	SDL_Rect square;
	int color = _color;
	
	clear();

	if (_popupStep > 0.0)
	{
		if (_popup == POPUP_HORIZONTAL || _popup == POPUP_BOTH)
		{
			square.x = (int)((_width - _width * _popupStep) / 2);
			square.w = (int)(_width * _popupStep);
		}
		else
		{
			square.x = 0;
			square.w = _width;
		}
		if (_popup == POPUP_VERTICAL || _popup == POPUP_BOTH)
		{
			square.y = (int)((_height - _height * _popupStep) / 2);
			square.h = (int)(_height * _popupStep);
		}
		else
		{
			square.y = 0;
			square.h = _height;
		}

		for (int i = 0; i < 5; i++)
		{
			SDL_FillRect(_surface, &square, color);
			if (i < 2)
				color--;
			else
				color++;
			square.x++;
			square.y++;
			square.w -= 2;
			square.h -= 2;
		}

		cropper.x = _x + square.x;
		cropper.y = _y + square.y;
		cropper.w = square.w;
		cropper.h = square.h;
		
		if (_bg != 0)
		{
			_bg->setCrop(&cropper);
			_bg->setX(square.x);
			_bg->setY(square.y);
			_bg->blit(this);
		}
	}
}

/**
 * Blits the bordered window on top of another surface.
 * @param surface Pointer to surface to blit onto.
 */
void Window::blit(Surface *surface)
{
	if (_popupStep >= 1.0)
		_popupStep = 1.0;
	else
	{
		_popupStep += 0.1;
		draw();
	}

	Surface::blit(surface);
}
