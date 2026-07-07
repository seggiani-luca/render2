#include "widget.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// -- input primitives

int hoverGui(guiContext *ctx, guiLayerId layId, float4 rect) {
  rect.y += ctx->layers[layId].vPos;
  return ctx->in.xCur >= rect.x && ctx->in.xCur <= rect.x + rect.z &&
         ctx->in.yCur >= rect.y && ctx->in.yCur <= rect.y + rect.w;
}

char *bufferGui(guiContext *ctx,
    guiLayerId layId,
    uint64_t id,
    float4 rect,
    int *submit,
    int *active) {
  *active = *submit = 0;

  // get if hover
  int hover = hoverGui(ctx, layId, rect);

  // activate on press
  if ((ctx->in.hotId == 0) && hover && ctx->in.curPress) {
    // set id
    ctx->in.hotId = id;

    // clear buffer
    memset(ctx->in.keyBuf, 0, IN_BUF_SIZ);
    ctx->in.keyBufSiz = 0;

    return NULL;
  }

  // check if active
  if (ctx->in.hotId != id)
    return NULL;
  *active = 1;

  // discard on any press
  if ((!hover && ctx->in.curPress) || ctx->in.escape) {
    // reset id
    ctx->in.hotId = 0;

    // if not empty, keep it (for ux)
    if (*ctx->in.keyBuf != '\0' && !ctx->in.escape)
      *submit = 1;

    // return temp. buffer
    return ctx->in.keyBuf;
  }

  // return on complete
  if (ctx->in.enter) {
    // reset id
    ctx->in.hotId = 0;

    // submit temp. buffer
    *submit = 1;
    return ctx->in.keyBuf;
  }

  // return temp. buffer
  return ctx->in.keyBuf;
}

void scrollGui(guiContext *ctx, guiLayerId layId, float min, float max) {
  // get absolute scroll
  ctx->in.absScroll += ctx->in.scroll * SCROLL_SENS;
  if (ctx->in.absScroll < min)
    ctx->in.absScroll = min;
  if (ctx->in.absScroll > max)
    ctx->in.absScroll = max;

  // scroll vPos
  ctx->layers[layId].vPos += ctx->in.absScroll;

  // consume scroll
  ctx->in.scroll = 0.0f;
}

// -- rendering primitives

void downGui(guiContext *ctx, guiLayerId layId, float amt) {
  ctx->layers[layId].vPos += amt;
}

void quadGui(guiContext *ctx, guiLayerId layId, float4 rect, float4 uv) {
  pushGui(&ctx->layers[layId],
      (quad){{rect.x, rect.y + ctx->layers[layId].vPos, rect.z, rect.w}, uv});
}

#define W 1.0f // width
void borderGui(guiContext *ctx, guiLayerId layId, float4 rect, float4 uv) {
  quadGui(ctx, layId, (float4){rect.x, rect.y, rect.z, W}, uv);
  quadGui(ctx, layId, (float4){rect.x, rect.y + rect.w - W, rect.z, W}, uv);
  quadGui(ctx, layId, (float4){rect.x, rect.y, W, rect.w}, uv);
  quadGui(ctx, layId, (float4){rect.x + rect.z - W, rect.y, W, rect.w}, uv);
}

void textGui(guiContext *ctx, guiLayerId layId, float2 pos, const char *str) {
  char c;
  int w = 0;

  // go through all characters
  while ((c = *str++)) {
    // extract character index
    int tX = (c - 32) % 32;
    int tY = (c - 32) / 32;

    // push characters
    pushGui(&ctx->layers[layId],
        (quad){
            {pos.x + w, ctx->layers[layId].vPos + pos.y, TXT_WIDTH, TXT_HEIGHT},
            {UV((float)tX / 2, tY), UV((float)tX / 2 + 0.5f, tY + 1.0f)}});

    // move right
    w += TXT_WIDTH;
  }
}

void iconGui(guiContext *ctx, guiLayerId layId, float2 pos, float4 uv) {
  pushGui(&ctx->layers[layId],
      (quad){{pos.x, pos.y + ctx->layers[layId].vPos, ICO_SIZ, ICO_SIZ}, uv});
}

int buttonGui(guiContext *ctx,
    guiLayerId layId,
    float4 rect,
    float4 ico,
    const char *str) {
  // push quad
  quadGui(ctx, layId, rect, BG_LIGHT);
  borderGui(ctx, layId, rect, FG_DARK);

  // display icon
  iconGui(ctx, layId, (float2){rect.x + 1 PAD, rect.y + 1 PAD}, ico);

  // display text
  textGui(ctx, layId, (float2){rect.x + 2 PAD + ICO_SIZ, rect.y + 1 PAD}, str);

  // check for input
  return hoverGui(ctx, layId, rect) && ctx->in.curReles;
}

void subWindowGui(guiContext *ctx, window *win) {
  ctx->child = win;
  ctx->inactive = 1;
  glfwMakeContextCurrent(ctx->win->gl); // hack for context
}

char *editorGui(guiContext *ctx,
    guiLayerId layId,
    float4 rect,
    const char *str,
    uint64_t id) {
  // check for input
  int active, submit;
  char *in = bufferGui(ctx, layId, id, rect, &submit, &active);

  // push quad
  quadGui(ctx, layId, rect, BG_DARK);
  borderGui(ctx, layId, rect, FG_DARK);

  // update displayed value and display
  textGui(ctx,
      layId,
      (float2){rect.x + 1 PAD, rect.y + 1 PAD},
      active ? in : str);

  // update actual value on submit
  if (submit)
    return in;
  return NULL;
}

void intGui(guiContext *ctx, guiLayerId layId, float4 rect, void *val) {
  // int to string
  char str[FIELD_SIZ];
  snprintf(str, FIELD_SIZ, "%d", *(int *)val);

  // update actual value on submit
  char *in = editorGui(ctx, layId, rect, str, (uint64_t)val + 1);
  if (in && val)
    *(int *)val = atoi(in);
}

void floatGui(guiContext *ctx, guiLayerId layId, float4 rect, void *val) {
  // int to string
  char str[FIELD_SIZ];
  snprintf(str, FIELD_SIZ, "%g", *(float *)val);

  // update actual value on submit
  char *in = editorGui(ctx, layId, rect, str, (uint64_t)val + 1);
  if (in && val)
    *(float *)val = atof(in);
}

void stringGui(guiContext *ctx, guiLayerId layId, float4 rect, void *val) {
  // int to string
  char str[FIELD_SIZ];
  snprintf(str, FIELD_SIZ, "%s", (char *)val);

  // update actual value on submit
  char *in = editorGui(ctx, layId, rect, str, (uint64_t)val + 1);
  if (in && val)
    strncpy(val, in, IN_BUF_SIZ);
}
